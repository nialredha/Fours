#include <stdio.h>
#include <assert.h>

#include "fours.h"
// TODO: include mixer.c here

#define NUM_STEPS (16)
#define MAX_CHARS (20)

#define MIN_BPM (20)
#define MAX_BPM (200)

char* sample_names[NUM_SAMPLES] = {"Kick", "HiHat", "Snare"};
char* sample_paths[NUM_SAMPLES] = {"../assets/kick.wav", "../assets/hihat.wav", "../assets/snare.wav"};

Sample samples[NUM_SAMPLES] = {0};
Track tracks[NUM_SAMPLES] = {0};

Button button = {0};
Slider slider = {0};

bool fours_init(Fours_State* state)
{
	// Initialize UI
	button = button_new_default(0, 0, NULL);
	slider = slider_new_default(0, 0);

    // Load Samples
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		samples[i].metadata.path = sample_paths[i];
		if (!load_sample(&samples[i]))
		{
			fprintf(stderr, "ERROR in %s, line %d: failed to load %s.\n", __FILE__, __LINE__, sample_paths[i]);
			return false;
		}
	}

	// Initialize Tracks
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		tracks[i].length = samples[i].length;
		tracks[i].buffer = samples[i].buffer; 
		tracks[i].volume = 1.0;
	}

	// Preallocate and Initialize Mix
	state->mix.metadata.sample_freq = samples[0].metadata.sample_freq;

    float seconds_per_beat = (1 / ((float)MIN_BPM / 60)) / 4;
    int samples_per_beat = (int)ceil(seconds_per_beat * state->mix.metadata.sample_freq);

    state->mix.length = samples_per_beat * NUM_STEPS; 
	state->mix.buffer = (float*)malloc(sizeof(float) * state->mix.length);
	clear_mix(&state->mix);

	state->mix.playhead = 0;
	state->mix.volume = 1.0;
	state->mix.num_tracks = NUM_SAMPLES;	
	state->mix.metadata.path = "";
	state->mix.metadata.channels = samples[0].metadata.channels;

	state->mouse.x = 0;
	state->mouse.y = 0;
    state->play_selected = false;
	for (int i = 0; i < NUM_PADS; i++)
	{
		state->pad_selected[i] = false;
	}
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		state->slider_selected[i] = false;
	}
	state->prev_bpm = 0;
	state->bpm = 60;
	state->bars = 16;
	state->samples_per_beat = 0;

	state->num_saves = 0;

	return true;
}

void fours_render_graphics(Fours_State* state) 
{
	// Play Button
	button.rect.x = 93; button.rect.y = 50; button.selected = &state->play_selected;
	if (add_button(&button))
	{
	    audio_toggle();
	}
	add_text("Play", button.center.x, 40);
	
	// BPM Variables
	char bpm_str[MAX_CHARS];
	
	// BPM Decrement Button
	button.rect.x += 68; button.selected = NULL;
	if (add_button(&button))
	{
		if (state->bpm > MIN_BPM) { state->bpm--; }
	}
	add_text("-", button.center.x, button.center.y);
	
	// BPM Display Name
	add_text("BPM", button.center.x, 40);
	
	// BPM Increment Button
	button.rect.x += 34; 
	if (add_button(&button))
	{
		if (state->bpm < MAX_BPM) { state->bpm++; }
	}
	add_text("+", button.center.x, button.center.y);
	
	// BPM Display Value
	snprintf(bpm_str, MAX_CHARS, "%d", state->bpm);
	add_text(bpm_str, button.center.x, 40);
	
	// Bars Variables
	char bars_str[MAX_CHARS];
	
	// Bars Decrement Button
	button.rect.x += 68; 
	if (add_button(&button))
	{
	    if (state->bars > 0) { state->bars--; }
	}
	add_text("-", button.center.x, button.center.y);
	
	// Bars Display Name
	add_text("Bars", button.center.x, 40);
	
	// Bars Increment Button
	button.rect.x += 34; 
	if (add_button(&button))
	{
	    if (state->bars < 16) { state->bars++; }
	}
	add_text("+", button.center.x, button.center.y);
	
	// Bars Display Value
	snprintf(bars_str, MAX_CHARS, "%d", state->bars);
	add_text(bars_str, button.center.x, 40);
	
	// Instrument Pads
	
	// add pads one row at a time
	button.rect.x = 93; button.rect.y = 100;
	int track_count = 0;
	for (int i = 0; i < NUM_SAMPLES*NUM_STEPS; i++)
	{
		button.selected = &state->pad_selected[i];
		add_button(&button);
		button.rect.x += button.rect.w + 10;
	
		// once a single row of pads are drawn,
		// add track title and move to next track
		if ((i+1) % NUM_STEPS == 0)
		{
			button.rect.x = 93;
			add_text(sample_names[track_count], 63, button.center.y);
			button.rect.y += button.rect.h + 10;
			track_count++;
		}
	}
	
	// add slider for each track
	slider.rect.x = 637; slider.rect.y = 100;
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
	    slider.rect.x += slider.rect.w + 2;
	    slider.fill.h = (int)(tracks[i].volume * slider.rect.h);
		slider.button.selected = &state->slider_selected[i];
	    float percent_full = add_slider(&slider);
	
	    if (state->slider_selected[i])
	    {
			tracks[i].volume = percent_full;
	    }
	}
	
	// Save Button
	button.rect.x = 603; button.rect.y = 215; button.selected = NULL;
	if (add_button(&button))
	{
		state->num_saves++;
		char file_name[MAX_CHARS];
		snprintf(file_name, MAX_CHARS, "%s%d%s", "track-", state->num_saves, ".wav");
		// TODO: actually export the audio
		state->mix.metadata.path = file_name; 
		export_mix_loop(&state->mix, state->bars);

	}
	add_text("Save", 573, button.center.y);

	return;
}

void fours_render_audio(Fours_State* state)
{
	if (state->prev_bpm != state->bpm)
	{
		float seconds_per_beat = (1 / ((float)state->bpm / 60)) / 4;
        state->samples_per_beat = (int)ceil(seconds_per_beat * state->mix.metadata.sample_freq);
        state->mix.length = state->samples_per_beat * NUM_STEPS; 
        state->mix.playhead = 0;
        state->prev_bpm = state->bpm;
	}

	clear_mix(&state->mix);
	
	for (int s = 0; s < NUM_SAMPLES; s++)
	{
		for (int step = 0; step < NUM_STEPS; step++)
		{
			int writehead = state->samples_per_beat * step;
			if (state->pad_selected[s * NUM_STEPS + step])
			{
				fill_mix(&tracks[s], &state->mix, writehead);
			}
		}
	}
}

void fours_close()
{
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		tracks[i].buffer = NULL;
		free(samples[i].buffer);
	}
}
