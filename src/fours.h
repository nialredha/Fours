#ifndef FOURS_H
#define FOURS_H

#define NUM_SAMPLES (3)
#define NUM_PADS (48)

#include <stdbool.h>

typedef struct 
{
    int x, y; 
} Position;

typedef struct 
{
	int width;
	int height;
	Position origin;
	Position center;
    bool* selected;
} Button;

typedef struct 
{
	int width;
	int height;
	int fill_height;
	Position origin;
    Button button;
	bool* drag;
} Slider;

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

	float* audio_buffer;
	int audio_length;
	int readhead;
} Fours_State;

// Rendering methods Fours provides to the Platform 
bool fours_init(Fours_State* state);
void fours_render_graphics(Fours_State* state);
void fours_render_audio(Fours_State* state);
void fours_close();

// Audio method Platform provides to Fours
void toggle_audio();

// UI methods Platform provides to Fours
bool add_button(Button* button);
float add_slider(Slider* slider);
bool add_text(char* text, int x, int y);

#endif // FOURS_H
