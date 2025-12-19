#pragma once

struct Song {
    const char* name;
    const int* melody;
    unsigned int length;   // number of (note, duration) pairs
    unsigned int tempo;
};
