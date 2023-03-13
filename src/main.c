#include <stdio.h>
#include <stdbool.h>

#include <SDL.h>

#include <audio.h>
#include <graphics.h>

#define NUM_TRACKS (3)
#define NUM_PADS (48)
#define MAX_CHARS (20)

char* track_names[NUM_TRACKS] = {"Kick", "HiHat", "Snare"};
char* track_paths[NUM_TRACKS] = {"../assets/kick.wav", "../assets/hihat.wav", "../assets/snare.wav"};

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        fprintf(stderr, "WARNING: input '%s', but program doesn't except input arguments.\n", argv[1]);
    }

    // SDL Audio Visual Initialization
    if (!graphics_init() | !audio_init()) 
    { 
        fprintf(stderr, "ERROR in %s, line %d: failed to initialize.\n", __FILE__, __LINE__);
        exit(1); 
    }

    // Audio Setup
	WAV_Track tracks[NUM_TRACKS] = {0};;
	for (int i = 0; i < NUM_TRACKS; i++)
	{
		tracks[i] = audio_load_track(track_paths[i]);
		if (tracks[i].buffer == NULL)
		{
			fprintf(stderr, "ERROR in %s, line %d: failed to load %s.\n", __FILE__, __LINE__, track_paths[i]);
        	exit(1); 
		}
	}

    // UI Setup
    Button button = button_new_default(0, 0, NULL);
    Slider slider = slider_new_default(0, 0);

    bool play_selected = false;
    bool pad_selected[NUM_PADS] = {false};
    bool slider_selected[NUM_TRACKS] = {false};

    SDL_Event event;
    Position mouse;

    // Main Loop
    bool quit = false;
    while (!quit)
    {
        SDL_GetMouseState(&mouse.x, &mouse.y);

        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) 
        { 
            audio_play(false);
            quit = true; 
        }
        else if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_SPACE) 
            { 
                play_selected = !play_selected; 
                audio_play(play_selected);
            }
        }
        else if (event.type == SDL_MOUSEBUTTONUP)
        {
            for (int i = 0; i < NUM_TRACKS; i++)
            {
                if (slider_selected[i])
                {
                    slider_selected[i] = false;
                }
            }
        }

        // Clear screen to gray
		SDL_Color color = {30, 30, 30, 255};
		graphics_clear_screen(color);

        // Render Frame

        // Play Button
        button.rect.x = 93; button.rect.y = 50; button.selected = &play_selected;
        if (add_button(&button, &mouse, &event))
        {
            audio_play(play_selected);
        }
		add_text("Play", button.center.x, 40);

        // BPM Variables
        static int bpm = 60;
        static char bpm_str[MAX_CHARS];

        // BPM Decrement Button
        button.rect.x += 68; button.selected = NULL;
        if (add_button(&button, &mouse, &event))
        {
			if (bpm > 0) { bpm--; }
        }
        add_text("-", button.center.x, button.center.y);

        // BPM Display Name
        add_text("BPM", button.center.x, 40);

        // BPM Increment Button
        button.rect.x += 34; 
        if (add_button(&button, &mouse, &event))
        {
			if (bpm < 200) { bpm++; }
        }
        add_text("+", button.center.x, button.center.y);

        // BPM Display Value
        snprintf(bpm_str, MAX_CHARS, "%d", bpm);
        add_text(bpm_str, button.center.x, 40);

        // Bars Variables
        static int bars = 0;
        static char bars_str[MAX_CHARS];

        // Bars Decrement Button
        button.rect.x += 68; 
        if (add_button(&button, &mouse, &event))
        {
            if (bars > 0) { bars--; }
        }
        add_text("-", button.center.x, button.center.y);

        // Bars Display Name
        add_text("Bars", button.center.x, 40);

        // Bars Increment Button
        button.rect.x += 34; 
        if (add_button(&button, &mouse, &event))
        {
            if (bars < 16) { bars++; }
        }
        add_text("+", button.center.x, button.center.y);

        // Bars Display Value
        snprintf(bars_str, MAX_CHARS, "%d", bars);
        add_text(bars_str, button.center.x, 40);

        // Instrument Pads

		// add pads one row at a time
        button.rect.x = 93; button.rect.y = 100;
		int track_count = 0;
		for (int i = 0; i < NUM_TRACKS*NUM_STEPS; i++)
		{
			button.selected = &pad_selected[i];
			add_button(&button, &mouse, &event);
			button.rect.x += button.rect.w + 10;

			// add track titles and move to next track
			if ((i+1) % NUM_STEPS == 0)
			{
				button.rect.x = 93;
				add_text(track_names[track_count], 63, button.center.y);
				button.rect.y += button.rect.h + 10;
				track_count++;
			}
		}

		// add slider for each track
        slider.rect.x = 637; slider.rect.y = 100;
        for (int i = 0; i < NUM_TRACKS; i++)
        {
            slider.rect.x += slider.rect.w + 2;
            slider.fill.h = (int)(tracks[i].volume * slider.rect.h);
			slider.button.selected = &slider_selected[i];
            float percent_full = add_slider(&slider, &mouse, &event);

            if (slider_selected[i])
            {
				tracks[i].volume = percent_full;
            }
        }

		// Fill mix buffer with current pad sequence
		for (int i = 0; i < NUM_TRACKS; i++)
		{
			audio_fill_mix(&tracks[i], &bpm, &pad_selected[NUM_STEPS*i]); 
		}

        // Save Button
        button.rect.x = 603; button.rect.y = 215; button.selected = NULL;
        if (add_button(&button, &mouse, &event))
        {
			static int track_num = 1;
        	char file_name[MAX_CHARS];
        	snprintf(file_name, MAX_CHARS, "%s%d%s", "track-", track_num, ".wav");

            if (!audio_export_wave(file_name, bars))
            {
                fprintf(stderr, "ERROR in %s, line %d: failed to export pattern.\n", __FILE__, __LINE__);
            }
            else { track_num++; }
        }
        add_text("Save", 573, button.center.y);
            
        // Present Frame
		graphics_display();
    }

    // Clean Up
	audio_close();
	for (int i = 0; i < NUM_TRACKS; i++) { audio_delete_track(&tracks[i]); }

	graphics_close();

    return 0;
}
