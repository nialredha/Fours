#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>

#include "mixer.h"
#include "wave.h"

void clear_mix(Mix* mix)
{
	// clear the mix buffer
	for (int n = 0; n < mix->length; n++)
    {
		mix->buffer[n] = 0.0;
    }
}

void fill_mix(Track* track, Mix* mix, int writehead)
{
    assert(track != NULL && mix != NULL);
	assert(writehead <= mix->length);

	for (int n = 0; n < track->length; n++)
	{
		if (writehead >= mix->length) 
		{ 
			writehead = 0;
			// break; 
		}
		float sample = (float)track->buffer[n] * track->volume;
		mix->buffer[writehead] += sample;
		writehead++;
	}
}

bool export_mix(Mix* mix)
{
	assert(mix != NULL);
	assert(mix->metadata.path != NULL && mix->buffer != NULL);

    uint32_t samples = (uint32_t)mix->length;
	int16_t* buffer = (int16_t*)mix->buffer;

    Wave wave = wave_construct(mix->metadata.path, mix->metadata.channels, 
								mix->metadata.sample_freq, samples, buffer);
    if (!wave_write(&wave)) { return false; }

    wave.data_buffer = NULL;

    return true;
}

bool export_mix_loop(Mix* mix, int bars)
{
	assert(mix != NULL);

	uint32_t samples = mix->length * bars;
	int16_t* looped_mix = (int16_t*)malloc(sizeof(int16_t) * samples);

	for (int b = 0; b < bars; b++)
	{
		for (int n = 0; n < mix->length; n++)
		{
			looped_mix[n + (b * mix->length)] = (int16_t)mix->buffer[n];
		}
	}

    Wave wave = wave_construct(mix->metadata.path, mix->metadata.channels, 
								mix->metadata.sample_freq, samples, looped_mix);
    if (!wave_write(&wave)) { return false; }

    wave.data_buffer = NULL;

    return true;
}

bool load_sample(Sample* sample)
{
	assert(sample->metadata.path != NULL);

	Wave wave;
	wave.path = sample->metadata.path;
	if(!wave_load(&wave)) { return false; }

	sample->buffer = wave.data_buffer;	
	sample->length = wave.num_samples;
	sample->metadata.channels = wave.num_channels;
	sample->metadata.sample_freq = wave.sample_rate;

	return true;
}
