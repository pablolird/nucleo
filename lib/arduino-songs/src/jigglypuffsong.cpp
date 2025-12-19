#include "jigglypuffsong.h"
#include "pitches.h"

static const int jigglypuffsong_melody[] = {
// Jigglypuff's Song
  // Score available at https://musescore.com/user/28109683/scores/5044153
  
  NOTE_D5,-4, NOTE_A5,8, NOTE_FS5,8, NOTE_D5,8,
  NOTE_E5,-4, NOTE_FS5,8, NOTE_G5,4,
  NOTE_FS5,-4, NOTE_E5,8, NOTE_FS5,4,
  NOTE_D5,-2,
  NOTE_D5,-4, NOTE_A5,8, NOTE_FS5,8, NOTE_D5,8,
  NOTE_E5,-4, NOTE_FS5,8, NOTE_G5,4,
  NOTE_FS5,-1,
  NOTE_D5,-4, NOTE_A5,8, NOTE_FS5,8, NOTE_D5,8,
  NOTE_E5,-4, NOTE_FS5,8, NOTE_G5,4,
  
  NOTE_FS5,-4, NOTE_E5,8, NOTE_FS5,4,
  NOTE_D5,-2,
  NOTE_D5,-4, NOTE_A5,8, NOTE_FS5,8, NOTE_D5,8,
  NOTE_E5,-4, NOTE_FS5,8, NOTE_G5,4,
  NOTE_FS5,-1,
};

const Song jigglypuffsong_song = {
    "Jigglypuffsong",
    jigglypuffsong_melody,
    sizeof(jigglypuffsong_melody) / sizeof(jigglypuffsong_melody[0]) / 2,
    85
};
