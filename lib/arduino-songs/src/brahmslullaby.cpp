#include "brahmslullaby.h"
#include "pitches.h"

static const int brahmslullaby_melody[] = {
// Wiegenlied (Brahms' Lullaby)
  // Score available at https://www.flutetunes.com/tunes.php?id=54

  NOTE_G4, 4, NOTE_G4, 4, //1
  NOTE_AS4, -4, NOTE_G4, 8, NOTE_G4, 4,
  NOTE_AS4, 4, REST, 4, NOTE_G4, 8, NOTE_AS4, 8,
  NOTE_DS5, 4, NOTE_D5, -4, NOTE_C5, 8,
  NOTE_C5, 4, NOTE_AS4, 4, NOTE_F4, 8, NOTE_G4, 8,
  NOTE_GS4, 4, NOTE_F4, 4, NOTE_F4, 8, NOTE_G4, 8,
  NOTE_GS4, 4, REST, 4, NOTE_F4, 8, NOTE_GS4, 8,
  NOTE_D5, 8, NOTE_C5, 8, NOTE_AS4, 4, NOTE_D5, 4,

  NOTE_DS5, 4, REST, 4, NOTE_DS4, 8, NOTE_DS4, 8, //8
  NOTE_DS5, 2, NOTE_C5, 8, NOTE_GS4, 8,
  NOTE_AS4, 2, NOTE_G4, 8, NOTE_DS4, 8,
  NOTE_GS4, 4, NOTE_AS4, 4, NOTE_C5, 4,
  NOTE_AS4, 2, NOTE_DS4, 8, NOTE_DS4, 8,
  NOTE_DS5, 2, NOTE_C5, 8, NOTE_GS4, 8,
  NOTE_AS4, 2, NOTE_G4, 8, NOTE_DS4, 8,
  NOTE_AS4, 4, NOTE_G4, 4, NOTE_DS4, 4,
  NOTE_DS4, 2
};

const Song brahmslullaby_song = {
    "Brahmslullaby",
    brahmslullaby_melody,
    sizeof(brahmslullaby_melody) / sizeof(brahmslullaby_melody[0]) / 2,
    76
};
