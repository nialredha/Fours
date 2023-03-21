#include <stdio.h>
#include <assert.h>

#include "audio.h"
#include "wave.h"

typedef struct
{
    float* buffer;
    int playhead;
    int length;
    int num_tracks;
    SDL_AudioSpec spec;
    SDL_AudioDeviceID device_id;
} Mix;

Mix mix = {0};

void audio_callback(void* userdata, Uint8* stream, int length)
{
    int16_t* stream16 = (int16_t*)stream;
    int length16 = length / BYTES_PER_SAMPLE; 
    userdata = NULL; // not sure how to make use of this
    
    // clear buffer to 0
    for (int i = 0; i < length16; i++)
    {
        stream16[i] = 0;
    }

    // fill buffer with mixed audio
    for (int i = 0; i < length16; i++)
    {
        if (mix.playhead >= mix.length) 
        { 
            mix.playhead = 0; 
        }
        stream16[i] = (int16_t)mix.buffer[mix.playhead];
        mix.playhead += 1;
    }
}

bool audio_init()
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
		return false;
	}

	mix.spec.callback = audio_callback;
    mix.device_id = SDL_OpenAudioDevice(NULL, 0, &mix.spec, NULL, 0);
    if (mix.device_id == 0)
    {
        fprintf(stderr, "SDL_OpenAudioDevice Error: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

WAV_Track audio_load_track(char* path)
{

    WAV_Track track; 

    track.path = path;
    track.volume = 1.0;
    track.length = 0;
    track.buffer = NULL;

    Uint8* buffer = NULL;
    Uint32 length = 0;

    if (SDL_LoadWAV(track.path, &mix.spec, &buffer, &length) == NULL)
    {
        fprintf(stderr, "SDL_LoadWAV Error: %s\n", SDL_GetError());
        return track;
    }
    track.buffer = (int16_t*)buffer;
    track.length = (int)(length / BYTES_PER_SAMPLE);

    track.volume = 1.0;

	mix.num_tracks++;

    return track;
}

void audio_delete_track(WAV_Track* track)
{
	track->path = NULL;
	SDL_FreeWAV((Uint8*)track->buffer); 
}

// TODO: generalize this for a sequencer and add another function that simply
// fills the mix with a track. I wonder how API would look for filling mix at
// a certain position would look like?
void audio_fill_mix(WAV_Track* track, int* bpm, bool* sequence)
{
    assert(track != NULL);
    assert(sequence != NULL);

    static int prev_bpm = 0;
    static int samples_per_beat = 0;
    static int track_number = 0;
    float seconds_per_beat = 0 ;

    SDL_LockAudioDevice(mix.device_id);
    
    // reallocate mix buffer
    if (prev_bpm != *bpm)
    {
        if (mix.buffer != NULL) { free(mix.buffer); }

		// TODO: generalize for different time signatures
        seconds_per_beat = (1 / ((float)*bpm / 60)) / 4;
        samples_per_beat = (int)ceil(seconds_per_beat * mix.spec.freq);
        mix.length = samples_per_beat * NUM_STEPS; 
        
        mix.buffer = (float*)malloc(sizeof(float) * mix.length);
        mix.playhead = 0;

        prev_bpm = *bpm;
        track_number = 0;
    }

    // clear the mix buffer
    if (track_number == 0)
    {
        for (int i = 0; i < mix.length; i++)
        {
            mix.buffer[i] = 0.0;
        }
        track_number = mix.num_tracks;
    }

    // fill mix buffer with track
    int writehead = 0;
    for (int i = 0; i < NUM_STEPS; i++)
    {
        for (int n = 0; n < track->length; n+=1)
        {
            writehead = n + (samples_per_beat*i);
            if (writehead >= mix.length) 
            { 
                writehead = 0; 
                break;  // prevent long samples from wrapping 
            }
            if (sequence[i])
            {
                float sample = (float)track->buffer[n] * track->volume;
                mix.buffer[writehead] += sample;
            }
        }
    }
    track_number--;

    SDL_UnlockAudioDevice(mix.device_id);
}

void audio_play(bool play)
{
    if (play) { SDL_PauseAudioDevice(mix.device_id, 0); }

    else 
	{ 
		SDL_PauseAudioDevice(mix.device_id, 1); 
		mix.playhead = 0;
	}
}

bool audio_export_wave(char* path, int bars)
{
    assert(path != NULL);

    uint16_t channels = (uint16_t)mix.spec.channels;
    uint32_t freq = (uint32_t)mix.spec.freq;
    uint32_t samples = (uint32_t)mix.length;

	// TODO: get rid of this malloc
    int16_t* final = (int16_t*)malloc(sizeof(int16_t) * samples * bars);
    if (final == NULL) { return false; }

    for (int i = 0; i < bars; i++)
    {
        for (uint32_t j = 0; j < samples; j++)
        {
            final[j+(i*samples)] = (int16_t)mix.buffer[j];
        }
    }
    samples *= bars;

    Wave wave = wave_new(path, channels, freq, samples, final);
    if (!wave_write(&wave)) { return false; }

    free(final);
    wave.data_buffer = NULL;

    return true;
}

void audio_close()
{
    SDL_CloseAudioDevice(mix.device_id);
    free(mix.buffer);
}
