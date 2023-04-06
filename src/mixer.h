#ifndef MIXER_H
#define MIXER_H

#include <stdint.h>
#include <stdbool.h>

// TODO: move all of this to the mixer.c file.
typedef struct
{
	char* path;	
	uint16_t channels;
	uint32_t sample_freq;
} Metadata;

typedef struct
{
    float* buffer; // TODO: update msvc to use Atomics 
    int length;

	// TODO: change to read head
    int playhead;	
    float volume;

    int num_tracks;

	Metadata metadata;
} Mix;

typedef struct 
{
    int16_t* buffer;
    int length;

    float volume;
} Track;

typedef struct 
{
    int16_t* buffer;
    int length;

	Metadata metadata;
} Sample;


void clear_mix(Mix* mix);
void fill_mix(Track* track, Mix* mix, int writehead);
bool export_mix(Mix* mix);
bool export_mix_loop(Mix* mix, int bars);

bool load_sample(Sample* sample);

#endif // MIXER_H
