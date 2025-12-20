#include "song_list.h"

struct Note {
  int frequency;
  float durationMs;
};

inline int songGetNoteFreq(const Song& song, int noteIdx) {
  int frequency = song.melody[noteIdx * 2];
  return frequency;
}
inline float songGetNoteDuration(const Song& song, int noteIdx) {
  int divider = song.melody[noteIdx * 2 + 1];
  unsigned long wholenote = (60000 * 4) / song.tempo;
  float duration =
      (divider > 0) ? ((float)wholenote / divider) : ((float)wholenote / abs(divider)) * 1.5;
  // Add spacing between consecutive notes
  duration *= 0.9;
  return duration;
}

Note songGetNote(const Song& song, int noteIdx) {
  Note note;
  note.frequency = songGetNoteFreq(song, noteIdx);
  note.durationMs = songGetNoteDuration(song, noteIdx);
  return note;
}

void songCumSum(const Song& song, float* out) {
  float cum = 0.0f;
  out[0] = cum;

  for (unsigned int i = 1; i < song.length; i++) {
    out[i] = cum + songGetNoteDuration(song, i-1);
    cum = out[i];
  }
}
