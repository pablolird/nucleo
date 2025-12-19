#include "happybirthday.h"
#include "pitches.h"

static const int happybirthday_melody[] = {
// Happy Birthday
  // Score available at https://musescore.com/user/8221/scores/26906

  NOTE_C4,4, NOTE_C4,8, 
  NOTE_D4,-4, NOTE_C4,-4, NOTE_F4,-4,
  NOTE_E4,-2, NOTE_C4,4, NOTE_C4,8, 
  NOTE_D4,-4, NOTE_C4,-4, NOTE_G4,-4,
  NOTE_F4,-2, NOTE_C4,4, NOTE_C4,8,

  NOTE_C5,-4, NOTE_A4,-4, NOTE_F4,-4, 
  NOTE_E4,-4, NOTE_D4,-4, NOTE_AS4,4, NOTE_AS4,8,
  NOTE_A4,-4, NOTE_F4,-4, NOTE_G4,-4,
  NOTE_F4,-2,
};

const Song happybirthday_song = {
    "Happybirthday",
    happybirthday_melody,
    sizeof(happybirthday_melody) / sizeof(happybirthday_melody[0]) / 2,
    140
};
