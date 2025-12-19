#include "song_list.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

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

// Map note frequency to bar position (0-15)
int noteToBar(int note) {
  if (note == 0)
    return -1; // REST
  // Simple frequency-based mapping (31 Hz to 4978 Hz covers most musical notes)
  return map(note, 31, 4978, 0, NUM_BANDS - 1);
}
void updateVisualizer(int currentNote) {
  // Decay all bars
  for (int i = 0; i < NUM_BANDS; i++) {
    if (visualBands[i] > 0)
      visualBands[i] -= 15;
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

void playSong(int songIndex) {
  if (songIndex >= song_count)
    return;

  const Song &song = *all_songs[songIndex];
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
  u8g2.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(20, 32, "Music Player");
  u8g2.sendBuffer();
  delay(1500);
}

void loop() {
  playSong(currentSong);

  currentSong++;
  if (currentSong >= song_count) {
    currentSong = 0;
  }

  delay(1000);
}
