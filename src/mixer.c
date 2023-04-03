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
		if (writehead >= mix->length) { break; }
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
