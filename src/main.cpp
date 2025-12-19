#include "song_list.h"
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <U8g2lib.h>
#include <Wire.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
LiquidCrystal_I2C *lcd = nullptr;

#define BUZZER_PIN D3
#define JOYSTICK_X_PIN A0
#define REST 0
#define NUM_BANDS 16
#define BAR_MIN_HEIGHT 25

const Song *const *songs = all_songs;
extern const unsigned int song_count;

int currentSong = 0;
uint8_t visualBands[NUM_BANDS] = {0};

// Joystick state tracking
unsigned long lastSkipTime = 0;
const unsigned long SKIP_INTERVAL = 1500; // 1.5 seconds in milliseconds
bool joystickHeld = false;
int lastJoystickDirection = 0; // 0 = neutral, 1 = right, -1 = left
int skipDirection = 0; // 0 = no skip, 1 = next, -1 = previous

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
  // Fast decay for ALL bars (including neighbors)
  for (int i = 0; i < NUM_BANDS; i++) {
    if (visualBands[i] > BAR_MIN_HEIGHT) {
      visualBands[i] -= 50; // Fast decay applies to everyone
      if (visualBands[i] < BAR_MIN_HEIGHT) {
        visualBands[i] = BAR_MIN_HEIGHT;
      }
    } else {
      visualBands[i] = BAR_MIN_HEIGHT;
    }
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

  // Draw bars
  u8g2.clearBuffer();
  int barWidth = 128 / NUM_BANDS;

  for (int i = 0; i < NUM_BANDS; i++) {
    int barHeight = map(visualBands[i], 0, 255, 0, 64);
    int x = i * barWidth;
    int y = 64 - barHeight;
    u8g2.drawBox(x, y, barWidth - 1, barHeight);
  }

  u8g2.sendBuffer();
}

void updateSongDisplay(int songIndex) {
  if (lcd && songIndex < song_count) {
    const Song &song = *all_songs[songIndex];
    lcd->setCursor(0, 0);
    // Print "Current Song (X/41)" - need to fit in 16 chars
    // "Current Song (X/41)" is too long, so use "Cur Song (X/41)" or similar
    String songCounter = "(" + String(songIndex + 1) + "/" + String(song_count) + ")";
    String displayText = "Song " + songCounter + ":";
    // Pad with spaces to clear any remaining characters
    while (displayText.length() < 16) {
      displayText += " ";
    }
    // Truncate if needed to fit 16 characters
    if (displayText.length() > 16) {
      displayText = displayText.substring(0, 16);
    }
    lcd->print(displayText);
    
    lcd->setCursor(0, 1);
    // Clear the line and print song name (max 16 chars)
    lcd->print("                "); // Clear line
    lcd->setCursor(0, 1);
    if (song.name) {
      // Truncate to 16 characters if needed
      String songName = String(song.name);
      if (songName.length() > 16) {
        songName = songName.substring(0, 16);
      }
      lcd->print(songName);
    }
  }
}

int checkJoystick() {
  int joystickX = analogRead(JOYSTICK_X_PIN);
  int currentDirection = 0;
  
  if (joystickX > 990) {
    currentDirection = 1; // Right
  } else if (joystickX < 20) {
    currentDirection = -1; // Left
  } else {
    currentDirection = 0; // Neutral
  }
  
  if (currentDirection != 0) {
    unsigned long currentTime = millis();
    
    // If direction changed or first press, skip immediately
    if (currentDirection != lastJoystickDirection || !joystickHeld) {
      lastSkipTime = currentTime;
      joystickHeld = true;
      return currentDirection; // Return direction for immediate skip
    } else if (joystickHeld && (currentTime - lastSkipTime >= SKIP_INTERVAL)) {
      // Timer-based skipping while held
      lastSkipTime = currentTime;
      return currentDirection; // Return direction for timer-based skip
    }
  } else {
    // Joystick released
    joystickHeld = false;
  }
  
  lastJoystickDirection = currentDirection;
  return 0; // No skip
}

void playSong(int songIndex) {
  if (songIndex >= song_count)
    return;

  const Song &song = *all_songs[songIndex];

  // Update LCD with current song name
  updateSongDisplay(songIndex);

  int wholenote = (60000 * 4) / song.tempo;

  for (unsigned int i = 0; i < song.length * 2; i += 2) {
    // Check joystick input before playing note
    skipDirection = checkJoystick();
    if (skipDirection != 0) {
      noTone(BUZZER_PIN);
      return; // Exit function to skip song
    }

    int divider = song.melody[i + 1];
    int duration =
        (divider > 0) ? wholenote / divider : (wholenote / abs(divider)) * 1.5;
    int currentNote = song.melody[i];

    // Play note
    if (currentNote != 0) {
      tone(BUZZER_PIN, currentNote, duration * 0.9);
    }

    // Update visualizer during note
    unsigned long noteStart = millis();
    while (millis() - noteStart < duration) {
      // Check joystick during note playback
      skipDirection = checkJoystick();
      if (skipDirection != 0) {
        noTone(BUZZER_PIN);
        return; // Exit function to skip song
      }
      
      updateVisualizer(currentNote);
      delay(20); // Refresh rate
    }

    noTone(BUZZER_PIN);
  }
  
  // Song completed normally, reset skip direction
  skipDirection = 0;
}

void setup() {
  Wire.begin();

  // Initialize OLED
  u8g2.begin();

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

  // Display startup message on OLED
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(20, 32, "Music Player");
  u8g2.sendBuffer();
  delay(1500);

  // Update LCD with first song
  if (lcd) {
    updateSongDisplay(0);
  }
}

void loop() {
  playSong(currentSong);

  // Handle navigation based on skip direction or normal progression
  if (skipDirection == 1) {
    // Skip to next song
    currentSong++;
    if (currentSong >= song_count) {
      currentSong = 0;
    }
    // Reset joystick state
    lastJoystickDirection = 0;
    joystickHeld = false;
    skipDirection = 0;
    delay(200); // Debounce delay
  } else if (skipDirection == -1) {
    // Go to previous song
    currentSong--;
    if (currentSong < 0) {
      currentSong = song_count - 1;
    }
    // Reset joystick state
    lastJoystickDirection = 0;
    joystickHeld = false;
    skipDirection = 0;
    delay(200); // Debounce delay
  } else {
    // No joystick input, advance normally
    currentSong++;
    if (currentSong >= song_count) {
      currentSong = 0;
    }
    delay(1000);
  }
}
