#include "thelick.h"
#include "pitches.h"

static const int thelick_melody[] = {
// The Lick 
  NOTE_D4,8, NOTE_E4,8, NOTE_F4,8, NOTE_G4,8, NOTE_E4,4, NOTE_C4,8, NOTE_D4,1,
};

const Song thelick_song = {
    "Thelick",
    thelick_melody,
    sizeof(thelick_melody) / sizeof(thelick_melody[0]) / 2,
    108
};
