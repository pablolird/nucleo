#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include "song_list.h"

LiquidCrystal_PCF8574 lcd(0x27);

#define BUZZER_PIN D3
#define BUTTON_PIN D2
#define SCROLL_DELAY 300
#define LCD_COLS 16
#define DEBOUNCE_DELAY 50
#define REST 0

const Song* const* songs = all_songs;
extern const unsigned int song_count;

int currentSong = 0;
int scrollPos = 0;
unsigned long lastScroll = 0;
bool skipRequested = false;
bool isPlaying = false;

// Button debouncing
volatile bool buttonPressed = false;
unsigned long lastButtonTime = 0;

void buttonISR() {
  buttonPressed = true;
}

void scrollTitle(const char* title) {
    int len = strlen(title);

    if (len <= LCD_COLS) {
        lcd.setCursor(0, 1);
        lcd.print(title);
        for (int i = len; i < LCD_COLS; i++) {
            lcd.print(' ');
        }
        return;
    }

    if (millis() - lastScroll < SCROLL_DELAY) return;
    lastScroll = millis();

    lcd.setCursor(0, 1);
    for (int i = 0; i < LCD_COLS; i++) {
        int idx = scrollPos + i;
        if (idx < len) {
            lcd.print(title[idx]);
        } else {
            lcd.print(' ');
        }
    }

    scrollPos++;
    if (scrollPos > len) {
        scrollPos = 0;
        delay(600);
    }
}

void playSong(int songIndex) {
    if (songIndex >= song_count) return;

    isPlaying = true;
    scrollPos = 0;

    const Song& song = *all_songs[songIndex];
    int wholenote = (60000 * 4) / song.tempo;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Now Playing:");

    for (unsigned int i = 0; i < song.length * 2; i += 2) {

        if (buttonPressed) {
            buttonPressed = false;
            break;
        }

        scrollTitle(song.name);

        int divider = song.melody[i + 1];
        int duration = (divider > 0)
            ? wholenote / divider
            : (wholenote / abs(divider)) * 1.5;

        if (song.melody[i] != 0) {
            tone(BUZZER_PIN, song.melody[i], duration * 0.9);
        }

        delay(duration);
        noTone(BUZZER_PIN);
    }

    isPlaying = false;
}


void setup() {
    Wire.begin();
    lcd.begin(16, 2);
    lcd.setBacklight(255);

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Music Player");
    lcd.setCursor(0, 1);
    lcd.print("Ready!");
    delay(1500);
}

void loop() {
    playSong(currentSong);
    
    currentSong++;
    if (currentSong >= song_count) {
        currentSong = 0;
    }
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Next song...");
    delay(1000);
}