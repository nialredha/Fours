#ifndef FOURS_H
#define FOURS_H

#define NUM_SAMPLES (3)
#define NUM_PADS (48)

#include <stdbool.h>

// TODO: eventually remove this dependency
#include "sdl_fours.h"

#include "mixer.h"

typedef struct 
{
	Position mouse;

	bool play_selected;
	bool pad_selected[NUM_PADS];
	bool slider_selected[NUM_SAMPLES];

	int prev_bpm;
	int bpm;
	int bars;
	int samples_per_beat;

	int num_saves;

	Mix mix;	// TODO: should generalize this so it isn't dependent on mixer
				// looks like we only need, playhead, length, and the buffer
} Fours_State;

// Rendering methods Fours provides to the Platform 
bool fours_init(Fours_State* state);
void fours_render_graphics(Fours_State* state);
void fours_render_audio(Fours_State* state);
void fours_close();

// TODO: add all the UI functions that are defined in sdl_fours.h to here
// UI methods Platform provides to Fours

#endif // FOURS_H
