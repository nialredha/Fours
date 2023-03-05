#ifndef AUDIO_H
#define AUDIO_H

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <SDL.h>

#include "wave.h"

#define NUM_STEPS (16)
#define NUM_PADS (48)

#define MIN_BPM (20)
#define MAX_BPM (200)
#define MAX_VOLUME (128)

typedef struct 
{
    char* path; 
    float  volume;
    int length;
    int16_t* buffer;
} Track16;

typedef struct
{
    float* buffer;
    int playhead;
    int length;
    int num_tracks;
    SDL_AudioSpec spec;
    SDL_AudioDeviceID device_id;
} Mix16;

Track16 audio_new_track(char* path)
{
    Track16 track; 

    track.path = path;
    track.volume = MAX_VOLUME;
    track.length = 0;
    track.buffer = NULL;

    return track;
}

Mix16 audio_new_mix(int num_tracks)
{
    Mix16 mix = {0};
    
    mix.buffer = NULL;
    mix.num_tracks = num_tracks;

    return mix;
} 

void audio_fill_mix(Track16* track, Mix16* mix, int* bpm, bool* sequence, float volume)
{
    assert(track != NULL);
    assert(mix != NULL);
    assert(sequence != NULL);

    static int prev_bpm = 0;
    static int samples_per_beat = 0;
    static int track_number = 0;
    float seconds_per_beat = 0 ;

    // limit bpm 
    if (*bpm < MIN_BPM) { *bpm = MIN_BPM; }
    else if (*bpm > MAX_BPM) { *bpm = MAX_BPM; }

    SDL_LockAudioDevice(mix->device_id);
    
    // reallocate mix buffer
    if (prev_bpm != *bpm)
    {
        if (mix->buffer != NULL) { free(mix->buffer); }

        seconds_per_beat = (1 / ((float)*bpm / 60)) / 4;
        samples_per_beat = (int)ceil(seconds_per_beat * mix->spec.freq);
        mix->length = samples_per_beat * NUM_STEPS; 
        
        mix->buffer = (float*)malloc(sizeof(float) * mix->length);
        mix->playhead = 0;

        prev_bpm = *bpm;
        track_number = 0;
    }

    // clear the mix buffer
    if (track_number == 0)
    {
        for (int i = 0; i < mix->length; i++)
        {
            mix->buffer[i] = 0.0;
        }
        track_number = mix->num_tracks;
    }

    // fill mix buffer with track
    int writehead = 0;
    for (int i = 0; i < NUM_STEPS; i++)
    {
        for (int n = 0; n < track->length; n+=1)
        {
            writehead = n + (samples_per_beat*i);
            if (writehead >= mix->length) 
            { 
                writehead = 0; 
                break;  // prevent long samples from wrapping 
            }
            if (sequence[i])
            {
                float percent_volume = volume / MAX_VOLUME;
                float sample = (float)track->buffer[n] * percent_volume;
                mix->buffer[writehead] += sample;
            }
        }
    }
    track_number--;

    SDL_UnlockAudioDevice(mix->device_id);
}

bool audio_export_wave(char* path, int bars, Mix16* mix)
{
    assert(path != NULL);
    assert( mix != NULL);

    uint16_t channels = (uint16_t)mix->spec.channels;
    uint32_t freq = (uint32_t)mix->spec.freq;
    uint32_t samples = (uint32_t)mix->length;

    int16_t* final = (int16_t*)malloc(sizeof(int16_t) * samples * bars);
    if (final == NULL) { return false; }

    for (int i = 0; i < bars; i++)
    {
        for (uint32_t j = 0; j < samples; j++)
        {
            final[j+(i*samples)] = (int16_t)mix->buffer[j];
        }
    }
    samples *= bars;

    Wave wave = wave_new(path, channels, freq, samples, final);
    if (!wave_write(&wave)) { return false; }

    free(final);
    wave.data_buffer = NULL;

    return true;
}

bool audio_open(Mix16* mix)
{
    assert(mix != NULL);
    mix->device_id = SDL_OpenAudioDevice(NULL, 0, &mix->spec, NULL, 0);
    if (mix->device_id == 0)
    {
        fprintf(stderr, "SDL_OpenAudioDevice Error: %s\n", SDL_GetError());
        return false;
    }
    return true;
}

bool audio_load_track(Track16* track, Mix16* mix)
{
    assert(mix != NULL);
    if (track->buffer != NULL) { free(track->buffer); }

    Uint8* buffer = NULL;
    Uint32 length = 0;
    if (SDL_LoadWAV(track->path, &mix->spec, &buffer, &length) == NULL)
    {
        fprintf(stderr, "SDL_LoadWAV Error: %s\n", SDL_GetError());
        return false;
    }
    track->buffer = (int16_t*)buffer;
    track->length = (int)(length / BYTES_PER_SAMPLE);

    track->volume = MAX_VOLUME;

    return true;
}

void audio_play(bool play, Mix16* mix)
{
    assert(mix != NULL);

    if (play) { SDL_PauseAudioDevice(mix->device_id, 0); }

    else { SDL_PauseAudioDevice(mix->device_id, 1); }
}

#endif // AUDIO_H
