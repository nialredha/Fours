#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <SDL.h>

// TODO: remove this and its dependencies
#define NUM_STEPS (16)

typedef struct 
{
    char* path; 
    float  volume;
    int length;
    int16_t* buffer;
} WAV_Track;

void audio_callback(void* userdata, Uint8* stream, int length);

bool audio_init();

WAV_Track audio_load_track(char* path);

void audio_delete_track(WAV_Track* track);

void audio_fill_mix(WAV_Track* track, int* bpm, bool* sequence);

void audio_play(bool play);

bool audio_export_wave(char* path, int bars);

void audio_close();

#endif // AUDIO_H
