#include "princeigor.h"
#include "pitches.h"

static const int princeigor_melody[] = {
// Prince Igor - Polovtsian Dances, Borodin 
  NOTE_G4, 4, NOTE_G4, 4, NOTE_D5, -2,
  NOTE_C5, 8, NOTE_D5, 8, NOTE_AS4, 4, NOTE_A4, 8, NOTE_G4, 8,
  NOTE_A4, 8, NOTE_AS4, 8, NOTE_C5, 1,
  
  NOTE_D5, 4, NOTE_A4, 4, NOTE_G4, 8, NOTE_F4, 8,
  NOTE_D4, 4, NOTE_D4, 4, NOTE_G4, -2,
  NOTE_A4, 4, NOTE_G4, 4, NOTE_F4, 8, NOTE_E4, 8,
  
  NOTE_F4, 4, NOTE_E4, 4, NOTE_D4, 1,
  NOTE_E4, 4, NOTE_F4, 4, NOTE_A4, 4,
  NOTE_G4, 4, NOTE_G4, 4, NOTE_AS4, -2,

  NOTE_C5, 4, NOTE_AS4, 4, NOTE_A4, 8, NOTE_G4, 8,
  NOTE_A4, 4, NOTE_AS4, 4, NOTE_C5, -2,
  NOTE_CS5, 4, NOTE_C5, 4, NOTE_A4, 4,
  
  NOTE_CS5, 4, NOTE_CS4, 4, NOTE_F5, -2,
  NOTE_G5, 4, NOTE_F5, 4, NOTE_DS4, 8, NOTE_CS4, 8,
  NOTE_F5, 2, NOTE_C5, -2, 
  
  NOTE_AS4, 4, NOTE_C5, 4, NOTE_AS4, 8, NOTE_A4, 8,
  NOTE_G4, 4, NOTE_G4, 4, NOTE_AS4, 1,
  NOTE_C5, 4, NOTE_AS4, 4, NOTE_A4, 8, NOTE_G4, 8,  

  NOTE_F4, 4, NOTE_G4, 4, NOTE_A4, 1,
  NOTE_AS4, 4, NOTE_A4, 4, NOTE_F4, 4,
  NOTE_G4, 4, NOTE_G4, 4, NOTE_D5, -2,

  NOTE_C5, 4, NOTE_AS4, 4, NOTE_A4, 8, NOTE_G4, 8,
  NOTE_A4,-1, NOTE_A4,-1, REST,2,
  NOTE_G4, 4, NOTE_G4, 4, NOTE_D5, -2,
  
  NOTE_C5, 8, NOTE_D5, 8, NOTE_AS4, 4, NOTE_A4, 8, NOTE_G4, 8,
  NOTE_A4, 8, NOTE_AS4, 8, NOTE_C5, 1,
  NOTE_C5, 8, NOTE_D5, 8, NOTE_C5, 4, NOTE_AS4, 8, NOTE_A4, 8,
  
  
  NOTE_D4, 4, NOTE_D4, 4, NOTE_G4, -2,
  NOTE_G4, 8, NOTE_A4, 8, NOTE_G4, 4, NOTE_F4, 8, NOTE_E4, 8,
  NOTE_F4, 8, NOTE_E4, 8, NOTE_D4, -2,//1
  
  REST,4, NOTE_C5, 8, NOTE_D5, 8, NOTE_C5, 4, NOTE_AS4, 8, NOTE_A4, 8,
  NOTE_G4, 4, NOTE_G4, 4, NOTE_B4, -2,
  NOTE_C5, 4, NOTE_AS4, 4, NOTE_A4, 8, NOTE_G4, 8,  

  NOTE_A4, 4, NOTE_G4, 4, NOTE_F4, -1, REST,4,
  NOTE_G4, 4, NOTE_G4, 4, NOTE_D5, -2,
  NOTE_C5, 8, NOTE_D5, 8, NOTE_AS4, 4, NOTE_A4, 8, NOTE_G4, 8,
  
  NOTE_A4, 4, NOTE_G4, 4, NOTE_F4, -2,
  NOTE_A4, 4, NOTE_G4, 4, NOTE_F4, -2,
  NOTE_A4, 4, NOTE_G4, 4, NOTE_F4, -1,
};

const Song princeigor_song = {
    "Princeigor",
    princeigor_melody,
    sizeof(princeigor_melody) / sizeof(princeigor_melody[0]) / 2,
    110
};
