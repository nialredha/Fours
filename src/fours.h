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

	Mix mix;
} Fours_State;

bool fours_init(Fours_State* state);
void fours_render_graphics(Fours_State* state);
void fours_render_audio(Fours_State* state);
void fours_close();

#endif // FOURS_H
