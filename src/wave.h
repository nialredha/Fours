#ifndef WAVE_H
#define WAVE_H

/* 
	Reference:
	- https://ccrma.stanford.edu/courses/422-winter-2014/projects/WaveFormat/
	- http://kernelx.weebly.com/bitwise-operators-in-c.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#define BUFFER_SIZE (4)
#define BYTES_PER_SAMPLE (2)
#define	BITS_PER_BYTE (8)
#define PCM (1)

typedef struct
{
    char* path;
	FILE* fp;

	uint32_t chunk_size;
	uint32_t fmt_chunk_size; 
	uint16_t audio_format; 
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
	uint16_t bytes_per_sample;
	uint32_t data_chunk_size;

	uint32_t duration_sec;
	uint32_t num_samples;
	int16_t* data_buffer;
} Wave;

Wave wave_construct(char* path, uint16_t num_channels, uint32_t sample_rate, 
					uint32_t num_samples, int16_t* buffer)
{
    Wave wave;

    // user input
    wave.path = path;
    wave.num_channels = num_channels;
    wave.sample_rate = sample_rate;
    wave.num_samples = num_samples;
    wave.data_buffer = buffer;

    // defaults
    wave.fp = NULL;
    wave.fmt_chunk_size = 16;
    wave.audio_format = PCM;
    wave.bits_per_sample = BYTES_PER_SAMPLE * 8;
    wave.bytes_per_sample = BYTES_PER_SAMPLE;

	wave.duration_sec = wave.num_samples / wave.sample_rate;
    wave.byte_rate = wave.sample_rate * wave.num_channels * wave.bytes_per_sample;
    wave.block_align = wave.num_channels * wave.bytes_per_sample;
    wave.data_chunk_size = wave.num_channels * wave.num_samples * wave.bytes_per_sample;
    wave.chunk_size = wave.data_chunk_size + 36;

    return wave;
}

void fill_buffer_32(uint8_t* buffer, uint32_t value)
{
    assert(buffer != NULL);

	buffer[0] = (uint8_t)(value >> 0);
	buffer[1] = (uint8_t)(value >> 8);
	buffer[2] = (uint8_t)(value >> 16);
	buffer[3] = value >> 24;
}

void fill_buffer_16(uint8_t* buffer, uint16_t first_value, uint16_t second_value)
{
    assert(buffer != NULL);

	buffer[0] = (uint8_t)(first_value >> 0);
	buffer[1] = first_value >> 8;
	buffer[2] = (uint8_t)(second_value >> 0);
	buffer[3] = second_value >> 8;
}

void fill_buffer_str(uint8_t* buffer, char* name)
{
    assert(buffer != NULL);
    assert(name != NULL);

	buffer[0] = name[0];
	buffer[1] = name[1];
	buffer[2] = name[2];
	buffer[3] = name[3];
}

// WARNING: Parsing is over-simplified. There are not many wave files this big guy can handle.
bool wave_load(Wave* wave)
{
    assert(wave != NULL);

	uint8_t buffer[BUFFER_SIZE];

    wave->fp = fopen(wave->path, "rb");
	if (!wave->fp) 
	{ 
        fprintf(stderr, "ERROR in %s, line %d: Couldn't open file.\n", __FILE__, __LINE__);
		return false;
	}

	// RIFF Chunk Descriptor **************************************************

	// Chunk ID (big endian)
	fread(buffer, 1, 4, wave->fp);
	if(buffer[0] != 'R' || buffer[1] != 'I' || buffer[2] != 'F' || buffer[3] != 'F')
	{
        fprintf(stderr, "ERROR in %s, line %d: %s is not RIFF!\n", __FILE__, __LINE__, wave->path);
		return false;
	}

	// Chunk Size (little endian) - size of file minus chunk id and chunk size 
	fread(buffer, 1, 4, wave->fp);
	wave->chunk_size = buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0] << 0;

	// Format (big endian)
	fread(buffer, 1, 4, wave->fp);
	if(buffer[0] != 'W' || buffer[1] != 'A' || buffer[2] != 'V' || buffer[3] != 'E')
	{
        fprintf(stderr, "ERROR in %s, line %d: %s is not WAVE!\n", __FILE__, __LINE__, wave->path);
		return false;
	}

	// FMT Sub-Chunk **********************************************************

	while (true)
	{
		// Sub-Chunk-1 ID (big endian)
		fread(buffer, 1, 4, wave->fp);
		if (buffer[0] == 'f' || buffer[1] == 'm' || buffer[2] == 't')
		{
			// Sub-Chunk-1 Size (little endian)
			fread(buffer, 1, 4, wave->fp);

			wave->fmt_chunk_size = buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0] << 0;

			// Audio Format and Number of Channels (little endian)
			fread(buffer, 1, 4, wave->fp);

			wave->audio_format = buffer[1] << 8 | buffer[0] << 0; // expecting 1 a.k.a PCM
			wave->num_channels = buffer[3] << 8 | buffer[2] << 0;

			if(wave->audio_format != 1)
			{ 
    		    fprintf(stderr, "ERROR in %s, line %d: %s format is not PCM!\n", __FILE__, __LINE__, wave->path);
				return false;
			}

			// Sample Rate (little endian)
			fread(buffer, 1, 4, wave->fp);

			wave->sample_rate = buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0] << 0;

			// Byte Rate (little endian) = sample_rate * num_channels * bytes_per_sample
			fread(buffer, 1, 4, wave->fp);
			wave->byte_rate = buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0] << 0;

			// Block Align (num_channels * bytes_per_sample) and Bits Per Sample (little endian)
			fread(buffer, 1, 4, wave->fp);

			wave->block_align = buffer[1] << 8 | buffer[0] << 0;
			wave->bits_per_sample = buffer[3] << 8 | buffer[2] << 0;

			wave->bytes_per_sample = wave->bits_per_sample / BITS_PER_BYTE;

			break;
		}

		// skip over random sub-chunk in hopes of finding the fmt sub-chunk
		fread(buffer, 1, 4, wave->fp);
		uint32_t chunk_size = buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0] << 0;
		if (fseek(wave->fp, chunk_size, SEEK_CUR))
		{
    	    fprintf(stderr, "ERROR in %s, line %d: Can't find the 'fmt' sub-chunk...\n", __FILE__, __LINE__);
			return false;
		}
	}


	// Data Sub-Chunk *********************************************************

	while(1)
	{
		// Sub-Chunk-2 ID (big endian)
		fread(buffer, 1, 4, wave->fp);
		if (buffer[0] == 'd' || buffer[1] == 'a' || buffer[2] == 't' || buffer[3] == 'a')
		{
			// Sub-Chunk-2 Size (little endian)
			fread(buffer, 1, 4, wave->fp);

			wave->data_chunk_size = buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0] << 0;

			// Data
			wave->num_samples = wave->data_chunk_size / (wave->num_channels * wave->bytes_per_sample);
			wave->duration_sec = wave->num_samples / wave->sample_rate;

			wave->data_buffer = (int16_t*)malloc(sizeof(int16_t) * wave->num_samples);
			if (wave->data_buffer == NULL) 
			{ 
    		    fprintf(stderr, "ERROR in %s, line %d: Couldn't allocate memory...\n", __FILE__, __LINE__);
				return false;
			} 

			fread(wave->data_buffer, wave->bytes_per_sample, wave->num_samples, wave->fp);

			break;
		}

		// skip over random sub-chunk in hopes of finding the data sub-chunk
		fread(buffer, 1, 4, wave->fp);
		uint32_t chunk_size = buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0] << 0;
		if (fseek(wave->fp, chunk_size, SEEK_CUR))
		{
    	    fprintf(stderr, "ERROR in %s, line %d: Can't find the 'data' sub-chunk...\n", __FILE__, __LINE__);
			return false;
		}
	}

	fclose(wave->fp);
    return true;
}

// Write a 16-bit PCM (uncompressed) WAVE file
bool wave_write(Wave* wave)
{
    assert(wave != NULL);

	uint8_t buffer[BUFFER_SIZE];
	char* name;

    wave->fp = fopen(wave->path, "wb");
	if (!wave->fp) 
	{ 
        fprintf(stderr, "ERROR in %s, line %d: Couldn't open file.\n", __FILE__, __LINE__);
		return false;
	} 

	// RIFF Chunk Descriptor **************************************************

	// Chunk ID (big endian)
	name = "RIFF";
	fill_buffer_str(buffer, name);
	fwrite(buffer, 1, 4, wave->fp);

	// Chunk Size (little endian) - size of file minus chunk id and chunk size 
	fill_buffer_32(buffer, wave->chunk_size);
	fwrite(buffer, 1, 4, wave->fp);

	// Format (big endian)
	name = "WAVE";
	fill_buffer_str(buffer, name);
	fwrite(buffer, 1, 4, wave->fp);

	// FMT Sub-Chunk **********************************************************

	// Sub-Chunk-1 ID (big endian)
	name = "fmt ";
	fill_buffer_str(buffer, name);
	fwrite(buffer, 1, 4, wave->fp);

	// Sub-Chunk-1 Size (little endian)
	fill_buffer_32(buffer, wave->fmt_chunk_size);
	fwrite(buffer, 1, 4, wave->fp);

	// Audio Format and Number of Channels (little endian)
	fill_buffer_16(buffer, wave->audio_format, wave->num_channels);
	fwrite(buffer, 1, 4, wave->fp);

	// Sample Rate (little endian)
	fill_buffer_32(buffer, wave->sample_rate);
	fwrite(buffer, 1, 4, wave->fp);

	// Byte Rate (little endian) = sample_rate * num_channels * bytes_per_sample
	fill_buffer_32(buffer, wave->byte_rate);
	fwrite(buffer, 1, 4, wave->fp);

	// Block Align (num_channels * bytes_per_sample) and Bits Per Sample (little endian)
	fill_buffer_16(buffer, wave->block_align, wave->bits_per_sample);
	fwrite(buffer, 1, 4, wave->fp);

	// Data Sub-Chunk *********************************************************

	// Sub-Chunk-2 ID (big endian)
	name = "data";
	fill_buffer_str(buffer, name);
	fwrite(buffer, 1, 4, wave->fp);

	// Sub-Chunk-2 Size (little endian)
	fill_buffer_32(buffer, wave->data_chunk_size);
	fwrite(buffer, 1, 4, wave->fp);

	// Data
	if (wave->data_buffer == NULL) 
	{ 
        fprintf(stderr, "ERROR in %s, line %d: where's the data?.\n", __FILE__, __LINE__);
		return false;
	} 

	fwrite(wave->data_buffer, wave->bytes_per_sample, wave->num_samples, wave->fp);
	fclose(wave->fp);

    return true;
}

#endif // WAVE_H
