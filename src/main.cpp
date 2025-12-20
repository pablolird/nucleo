#include "clib/u8g2.h"
#include "song_list.h"
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "note.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
LiquidCrystal_I2C *lcd = nullptr;

#define BUZZER_PIN D3
#define JOYSTICK_X_PIN A0
#define JOYSTICK_Y_PIN A1
#define PAUSE_BUTTON_PIN D2
#define REST 0
#define NUM_BANDS 16
#define BAR_PIXEL_TOP_Y 2
#define BAR_PIXEL_BOT_Y 34

const Song *const *songs = all_songs;
extern const unsigned int song_count;

// UI
struct {
  bool isPaused = false;
  int currentSong = 0;
  int currentSpeedSettingIdx = 2;

  uint8_t visualBands[NUM_BANDS] = {0};

  int currentSongNoteIdx = 0;
  float *songNoteTimes;
  float songDuration;

  uint32_t scrollTimeBegin = 0;
} uiState;

// Joystick state tracking
const unsigned long SKIP_INTERVAL = 1500; // 1.5 seconds in milliseconds
unsigned long lastJoystickXFlick = 0;
int lastJoystickXDirection = 0; // 0 = neutral, 1 = right, -1 = left
unsigned long lastJoystickYFlick = 0;
int lastJoystickYDirection = 0; // 0 = neutral, 1 = up, -1 = down

// Button state tracking
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 50; // 50ms debounce delay
const unsigned long BUTTON_PRESS_COOLDOWN = 200; // 200ms between button presses

// Playback parameters (speedup, volume, pitch)
const float SPEED_SETTINGS[] = {0.25, 0.5, 1.0, 1.5, 2.0, 3.0};
const char* SPEED_SETTINGS_STR[] = {"0.25x", "0.5x", "1.0x", "1.5x", "2.0x", "3.0x"};
const int SPEED_SETTINGS_MAX_IDX = (sizeof(SPEED_SETTINGS) / sizeof(SPEED_SETTINGS[0]) - 1);

// Scan I2C bus for LCD address (address-independent)
uint8_t scanI2CForLCD() {
  // Common I2C addresses for LCD 1602 with PCF8574
  uint8_t addresses[] = {0x27, 0x3F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
                         0x26, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E};
  uint8_t numAddresses = sizeof(addresses) / sizeof(addresses[0]);

  for (uint8_t i = 0; i < numAddresses; i++) {
    Wire.beginTransmission(addresses[i]);
    if (Wire.endTransmission() == 0) {
      return addresses[i];
    }
  }
  return 0; // Not found
}

void toneWithVolume(uint8_t pin, unsigned int frequency, unsigned long duration, uint8_t dutyCycle = 50) {
  tone(pin, frequency, duration);
  return;
  // Constrain parameters
  if (frequency < 31) frequency = 31;
  dutyCycle = constrain(dutyCycle, 0, 100);
  
  // Calculate period in microseconds
  unsigned long period = 1000000UL / frequency;
  unsigned long highTime = (period * dutyCycle) / 100;
  unsigned long lowTime = period - highTime; 
  
  pinMode(pin, OUTPUT);
  
  // Generate tone for specified duration
  unsigned long endTime = millis() + duration;
  while (millis() < endTime) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(highTime);
    digitalWrite(pin, LOW);
    delayMicroseconds(lowTime);
  }
}

// Map note to bar based on musical scale position
int noteToBar(int note) {
  if (note == 0)
    return -1; // REST

  // Define note boundaries for each bar (divide the musical range evenly)
  // This covers the full range from NOTE_B0 (31) to NOTE_DS8 (4978)
  const int boundaries[NUM_BANDS + 1] = {
      31,   // NOTE_B0
      65,   // NOTE_C2
      98,   // NOTE_G2
      131,  // NOTE_C3
      165,  // NOTE_E3
      196,  // NOTE_G3
      247,  // NOTE_B3
      294,  // NOTE_D4
      349,  // NOTE_F4
      415,  // NOTE_GS4
      494,  // NOTE_B4
      587,  // NOTE_D5
      698,  // NOTE_F5
      831,  // NOTE_GS5
      988,  // NOTE_B5
      1319, // NOTE_E6
      4978  // NOTE_DS8
  };

  // Find which bar this note belongs to
  for (int i = 0; i < NUM_BANDS; i++) {
    if (note >= boundaries[i] && note < boundaries[i + 1]) {
      return i;
    }
  }

  return NUM_BANDS - 1; // Fallback to last bar
}

void updateVisualizer(int currentNote) {
  auto& visualBands = uiState.visualBands;

  // Fast decay for ALL bars (including neighbors)
  for (int i = 0; i < NUM_BANDS; i++) {
    visualBands[i] = constrain(visualBands[i] - 50, 25, 255);
  }

  // THEN add energy for current note and neighbors
  if (currentNote != REST) {
    int barIndex = noteToBar(currentNote);
    if (barIndex >= 0 && barIndex < NUM_BANDS) {
      // Main bar gets max energy
      visualBands[barIndex] = 255;

      // Immediate neighbors get a boost (not set to fixed value)
      if (barIndex > 0) {
        visualBands[barIndex - 1] = min(200, visualBands[barIndex - 1] + 100);
      }
      if (barIndex < NUM_BANDS - 1) {
        visualBands[barIndex + 1] = min(200, visualBands[barIndex + 1] + 100);
      }

      // Second neighbors get smaller boost
      if (barIndex > 1) {
        visualBands[barIndex - 2] = min(175, visualBands[barIndex - 2] + 50);
      }
      if (barIndex < NUM_BANDS - 2) {
        visualBands[barIndex + 2] = min(175, visualBands[barIndex + 2] + 50);
      }
    }
  }
}

void drawUI_oled() {
  const Song &song = *all_songs[uiState.currentSong];
  u8g2.clearBuffer();

  // Draw play status
  int baseY = 40;
  if (uiState.isPaused) {
    u8g2.drawBox(0, baseY, 4, 12);
    u8g2.drawBox(6, baseY, 4, 12);
  } else {
    u8g2.drawTriangle(0, baseY, 12, baseY + 6, 0, baseY + 12);
  }

  // Show speed
  int curMinutes = ((unsigned long)uiState.songNoteTimes[uiState.currentSongNoteIdx] / 1000) / 60;
  int curSeconds = ((unsigned long)uiState.songNoteTimes[uiState.currentSongNoteIdx] / 1000) % 60;
  int durationMinutes = ((unsigned long)uiState.songDuration / 1000) / 60;
  int durationSeconds = ((unsigned long)uiState.songDuration / 1000) % 60;
  auto speedStr = SPEED_SETTINGS_STR[uiState.currentSpeedSettingIdx];
  
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(16, 50);
  u8g2.printf("%02d:%02d/%02d:%02d (%s)", curMinutes, curSeconds, durationMinutes, durationSeconds, speedStr);
  
  // Song progress
  int progressX = (int)((uiState.songNoteTimes[uiState.currentSongNoteIdx] / uiState.songDuration) * 128);
  u8g2.drawBox(0, 59, progressX, 3);
  u8g2.drawBox(progressX, 60, 128, 1);
  u8g2.drawBox(progressX-1, 56, 2, 8);

  // Draw visualizer
  int barWidth = 128 / NUM_BANDS;
  for (int i = 0; i < NUM_BANDS; i++) {
    int barHeight = map(uiState.visualBands[i], 0, 255, 0,
    				  BAR_PIXEL_BOT_Y - BAR_PIXEL_TOP_Y);
    int x = i * barWidth;
    int y = BAR_PIXEL_BOT_Y - barHeight;
    u8g2.drawBox(x, y, barWidth - 1, barHeight);
  }

  u8g2.sendBuffer();
}

void drawUI_lcd() {
  if (lcd) {
    const Song &song = *all_songs[uiState.currentSong];
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->printf("Song (%d/%d):", uiState.currentSong+1, song_count);

    int songNameLength = strlen(song.name);

    lcd->setCursor(0, 1);
    if (songNameLength <= 16) {
      lcd->printf("%s", song.name);
    } else {
      uint32_t scrollTime = millis() - uiState.scrollTimeBegin;
      char songNameBuffer[17] = {};
      int offset = 0;

      const uint32_t T1 = 2000;
      const uint32_t T2 = 8000;

      if (scrollTime < 2000) {
        offset = 0;
      } else if (scrollTime >= T1 && scrollTime < T2) {
        offset = ((scrollTime - T1) * (songNameLength + 1)) / (T2 - T1);
      } else if (scrollTime >= T2) {
        uiState.scrollTimeBegin = millis();
      }

      for (int i = 0; i < 16; i++) {
        songNameBuffer[i] = song.name[(offset + i) % (songNameLength + 1)];
        if (songNameBuffer[i] == '\0')
          songNameBuffer[i] = ' ';
      }

      lcd->printf("%s", songNameBuffer);
    }
  }
}

void drawUI() {
  drawUI_lcd();
  drawUI_oled();
}

int checkJoystickX() {
  int joystickX = analogRead(JOYSTICK_Y_PIN);
  // Neutral by default
  int currentJoystickXDirection = 0;
  int flick = 0;
  
  if (joystickX > 950) {
    currentJoystickXDirection = 1; // Right
  } else if (joystickX < 250) {
    currentJoystickXDirection = -1; // Left
  }

  unsigned long currentTime = millis();

  // Only skip if direction changed OR the last skip was 1.5secs ago (timer-based skipping if held)
  if ((currentJoystickXDirection != lastJoystickXDirection) ||
	  (currentTime - lastJoystickXFlick >= SKIP_INTERVAL)) {
	  lastJoystickXFlick = currentTime;
	  flick = currentJoystickXDirection;
  }
  lastJoystickXDirection = currentJoystickXDirection;
  return flick;
}

int checkJoystickY() {
  int joystickY = analogRead(JOYSTICK_X_PIN);
  // Neutral by default
  int currentJoystickYDirection = 0;
  int flick = 0;
  
  if (joystickY > 950) {
    currentJoystickYDirection = 1; // Up
  } else if (joystickY < 250) {
    currentJoystickYDirection = -1; // Down
  }

  unsigned long currentTime = millis();

  // Only skip if direction changed OR the last skip was 1.5secs ago (timer-based skipping if held)
  if ((currentJoystickYDirection != lastJoystickYDirection) ||
	  (currentTime - lastJoystickYFlick >= SKIP_INTERVAL)) {
	  lastJoystickYFlick = currentTime;
	  flick = currentJoystickYDirection;
  }
  lastJoystickYDirection = currentJoystickYDirection;
  return flick;
}

bool checkPauseButton() {
  static unsigned long lastPressTime = 0;
  static int lastStableReading = HIGH;
  int reading = digitalRead(PAUSE_BUTTON_PIN);
  unsigned long currentTime = millis();
  
  // Simple debounce: if button state changed, reset debounce timer
  if (reading != lastButtonState) {
    lastDebounceTime = currentTime;
  }
  
  lastButtonState = reading;
  
  // Only process if debounce time has passed
  if ((currentTime - lastDebounceTime) > DEBOUNCE_DELAY) {
    // Detect button press (HIGH to LOW transition) with cooldown
    if (reading == LOW && lastStableReading == HIGH && 
        (currentTime - lastPressTime) > BUTTON_PRESS_COOLDOWN) {
      lastStableReading = LOW;
      lastPressTime = currentTime;
      return true; // Button was just pressed
    }
    
    // Update stable reading on release
    if (reading == HIGH) {
      lastStableReading = HIGH;
    }
  }
  
  return false;
}

int handlePauseOrSkipReq() {
    int skip = 0;
    bool isPaused = false;

    if ((skip = checkJoystickX()) != 0)
      return skip;
    if (checkPauseButton())
      isPaused = true;

    while (isPaused) {
      if ((skip = checkJoystickX()) != 0)
        return skip;
      // press again to unpause
      if (checkPauseButton())
        isPaused = false;

      // Update visualizer with rest (no note) while paused
      updateVisualizer(REST);
      uiState.isPaused = true;
      drawUI();
      delay(20); // Small delay to prevent tight loop
    }
    
    uiState.isPaused = false;
    return 0;
}

// Returns how many songs to skip
int playSong(int songIndex) {
  int skip = 0;
  int speedSettingIdx = 2;
  int speedChange = 0;

  const Song &song = *all_songs[songIndex];

  uiState.currentSong = songIndex;
  songCumSum(song, uiState.songNoteTimes);
  uiState.songDuration = uiState.songNoteTimes[song.length - 1];
  uiState.scrollTimeBegin = millis();
  uiState.currentSpeedSettingIdx = speedSettingIdx;
  uiState.isPaused = false;
  drawUI();

  for (unsigned int noteIdx = 0; noteIdx < song.length; noteIdx++) {
    uiState.currentSongNoteIdx = noteIdx;

    if ((skip = handlePauseOrSkipReq()) != 0)
      return skip;

    Note note = songGetNote(song, noteIdx);
    note.durationMs /= SPEED_SETTINGS[speedSettingIdx];
    if (note.frequency != 0) {
      tone(BUZZER_PIN, note.frequency, (unsigned long)note.durationMs);
    }

	// Update visualizer during note
    unsigned long noteStart = millis();
    while (millis() - noteStart < note.durationMs) {
      if ((skip = handlePauseOrSkipReq()) != 0)
        return skip;

      if ((speedChange = checkJoystickY()) != 0) {
        speedSettingIdx = constrain(speedSettingIdx + speedChange, 0, SPEED_SETTINGS_MAX_IDX);
        uiState.currentSpeedSettingIdx = speedSettingIdx;
      }

      updateVisualizer(note.frequency);
      drawUI();
      delay(20);
    }
  }

  return 1;
}

int mod(int a, int b) {
  return (a % b + b) % b;
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Scan for LCD 1602 I2C address
  uint8_t lcdAddress = scanI2CForLCD();
  if (lcdAddress != 0) {
    lcd = new LiquidCrystal_I2C(lcdAddress, 16, 2);
    lcd->init();
    lcd->backlight();
    // Display will be updated by updateSongDisplay
    lcd->setCursor(0, 1);
    lcd->print("Initializing...  ");
  }

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(JOYSTICK_X_PIN, INPUT);
  pinMode(JOYSTICK_Y_PIN, INPUT);
  pinMode(PAUSE_BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize button state
  lastButtonState = digitalRead(PAUSE_BUTTON_PIN);

  // Init OLED and show startup message
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(20, 32, "Music Player");
  u8g2.setCursor(20, 30);
  u8g2.sendBuffer();
  delay(800);

  int longest = 0;
  for (int i = 0; i < song_count; i++)
    if (songs[i]->length > longest)
      longest = songs[i]->length;

  uiState.songNoteTimes = (float*)malloc(longest * sizeof(float));

  int currentSong = 0;
  for (;;) {
    int skipDirection = playSong(currentSong);
    currentSong = mod(currentSong + skipDirection, song_count);
    delay(200);
  }
}

void loop() {}
