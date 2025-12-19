#include "song_list.h"
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <U8g2lib.h>
#include <Wire.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
LiquidCrystal_I2C *lcd = nullptr;

#define BUZZER_PIN D3
#define BUTTON_PIN D2
#define REST 0
#define NUM_BANDS 16

const Song *const *songs = all_songs;
extern const unsigned int song_count;

int currentSong = 0;
uint8_t visualBands[NUM_BANDS] = {0};

volatile bool buttonPressed = false;

void buttonISR() { buttonPressed = true; }

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
  // Faster decay - increase this number for even faster
  for (int i = 0; i < NUM_BANDS; i++) {
    if (visualBands[i] > 50) { // Decay to baseline of 50 instead of 0
      visualBands[i] -= 60;    // Faster decay (was 15)
    } else {
      visualBands[i] = 50; // Baseline level for all bars
    }
  }

  // Set current note bar to max
  if (currentNote != REST) {
    int barIndex = noteToBar(currentNote);
    if (barIndex >= 0 && barIndex < NUM_BANDS) {
      visualBands[barIndex] = 255;
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
    lcd->print("Current Song:    ");
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

void playSong(int songIndex) {
  if (songIndex >= song_count)
    return;

  const Song &song = *all_songs[songIndex];

  // Update LCD with current song name
  updateSongDisplay(songIndex);

  int wholenote = (60000 * 4) / song.tempo;

  for (unsigned int i = 0; i < song.length * 2; i += 2) {
    if (buttonPressed) {
      buttonPressed = false;
      break;
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
      updateVisualizer(currentNote);
      delay(20); // Refresh rate
    }

    noTone(BUZZER_PIN);
  }
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
    lcd->setCursor(0, 0);
    lcd->print("Current Song:    ");
    lcd->setCursor(0, 1);
    lcd->print("Initializing...  ");
  }

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

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

  currentSong++;
  if (currentSong >= song_count) {
    currentSong = 0;
  }

  delay(1000);
}
