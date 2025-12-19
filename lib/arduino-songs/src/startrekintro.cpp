#include "startrekintro.h"
#include "pitches.h"

static const int startrekintro_melody[] = {
// Star Trek Intro
  // Score available at https://musescore.com/user/10768291/scores/4594271
 
  NOTE_D4, -8, NOTE_G4, 16, NOTE_C5, -4, 
  NOTE_B4, 8, NOTE_G4, -16, NOTE_E4, -16, NOTE_A4, -16,
  NOTE_D5, 2,
};

const Song startrekintro_song = {
    "Startrekintro",
    startrekintro_melody,
    sizeof(startrekintro_melody) / sizeof(startrekintro_melody[0]) / 2,
    80
};
