#ifndef AUDIO_H
#define AUDIO_H

#include <SDL.h>
#include <math.h>
#include <stdio.h>

#define NUM_STEPS (16)
#define NUM_PADS (48)
#define BYTES_PER_SAMPLE (2)

#define MIN_BPM (20)
#define MAX_BPM (200)
#define MAX_VOLUME (128)

typedef struct 
{
    char* path; 
    int  volume;
    int length;
    Uint16* buffer;
} Track16;

typedef struct
{
    Uint16* buffer;
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

bool audio_fill_mix(Track16* track, Mix16* mix, int* bpm, bool* sequence)
{
    static int prev_bpm = 0;
    static int samples_per_beat = 0;
    static int track_number = 0;
    float seconds_per_beat = 0 ;

    // limit bpm 
    if (*bpm < MIN_BPM) { *bpm = MIN_BPM; }
    else if (*bpm > MAX_BPM) { *bpm = MAX_BPM; }

    SDL_LockAudioDevice(mix->device_id);
    
    if (track->buffer == NULL || sequence == NULL) 
    { 
        fprintf(stderr, "ERROR in %s, line %d: passed null pointer\n", __FILE__, __LINE__);
        return false;
    }

    if (prev_bpm != *bpm)
    {

        if (mix->buffer != NULL) { free(mix->buffer); }

        seconds_per_beat = 1 / (float)*bpm * 60;
        samples_per_beat = (int)ceil(seconds_per_beat * mix->spec.freq);
        mix->length = samples_per_beat * NUM_STEPS; 
        
        mix->buffer = (Uint16*)malloc(sizeof(Uint16) * mix->length);
        mix->playhead = 0;

        prev_bpm = *bpm;
        track_number = 0;
    }

    if (track_number == 0)
    {
        for (int i = 0; i < mix->length; i++)
        {
            mix->buffer[i] = 0;
        }
        track_number = mix->num_tracks;
    }

    for (int i = 0; i < NUM_STEPS; i++)
    {
        for (int n = 0; n < samples_per_beat; n+=1)
        {
            if (n < track->length && sequence[i])
            {
                mix->buffer[n+(samples_per_beat*i)] += track->buffer[n]; 
            }
        }
    }

    track_number--;

    SDL_UnlockAudioDevice(mix->device_id);

    return true;
}

bool audio_open(Mix16* mix)
{
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
    if (track->buffer != NULL) { free(track->buffer); }

    Uint8* buffer = NULL;
    Uint32 length = 0;
    if (SDL_LoadWAV(track->path, &mix->spec, &buffer, &length) == NULL)
    {
        fprintf(stderr, "SDL_LoadWAV Error: %s\n", SDL_GetError());
        return false;
    }
    track->buffer = (Uint16*)buffer;
    track->length = (int)(length / BYTES_PER_SAMPLE);

    track->volume = MAX_VOLUME;

    return true;
}

void audio_play(bool play, Mix16* mix)
{
    if (play) { SDL_PauseAudioDevice(mix->device_id, 0); }

    else { SDL_PauseAudioDevice(mix->device_id, 1); }
}

#endif // AUDIO_H
