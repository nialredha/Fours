#include <stdio.h>
#include <stdbool.h>

#include <SDL.h>

#include <audio.h>
#include <ui_tools.h>
#include <wave.h>

#define SCREEN_WIDTH (720)
#define SCREEN_HEIGHT (295)
#define NUM_TRACKS (3)
#define MAX_CHARS (20)

char* track_names[NUM_TRACKS] = {"Kick", "HiHat", "Snare"};
char* track_paths[NUM_TRACKS] = {"../assets/kick.wav", "../assets/hihat.wav", "../assets/snare.wav"};

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;
Mix16 mix = {0};

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

bool init_av()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }
    if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN, 
                                    &window, &renderer) < 0)
    {
        fprintf(stderr, "SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
        return false;
    }
    if (TTF_Init() < 0)
    {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        return false;
    }

    font = TTF_OpenFont("../assets/Roboto-Regular.ttf", 16);
	if (!font)
	{
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
		return false;
	}

    SDL_ShowCursor(SDL_ENABLE);

    return true;
}

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        fprintf(stderr, "WARNING: input '%s', but program doesn't except input arguments.\n", argv[1]);
    }

    // SDL Audio Visual Initialization
    if (!init_av()) 
    { 
        fprintf(stderr, "ERROR in %s, line %d: failed to initialize.\n", __FILE__, __LINE__);
        exit(1); 
    }

    // Button Setup
    Button button = button_new_default(0, 0, NULL);
    Slider slider = slider_new_default(0, 0);
    bool play_selected = false;
    bool pad_selected[NUM_PADS] = {false};
    bool slider_selected[NUM_TRACKS] = {false};

    // Audio Setup
    mix = audio_new_mix(NUM_TRACKS);
	Track16 tracks[NUM_TRACKS] = {0};;
	for (int i = 0; i < NUM_TRACKS; i++)
	{
		Track16 track = audio_new_track(track_paths[i]);
		if (!audio_load_track(&track, &mix))
		{
			fprintf(stderr, "ERROR in %s, line %d: failed to load %s.\n", __FILE__, __LINE__, track_paths[i]);
        	exit(1); 
		}
		tracks[i] = track;
	}

    mix.spec.callback = audio_callback;
    if(!audio_open(&mix)) 
    { 
        fprintf(stderr, "ERROR in %s, line %d: failed to open audio device.\n", __FILE__, __LINE__);
        exit(1); 
    } 

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
            audio_play(false, &mix);
            quit = true; 
        }
        else if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_SPACE) 
            { 
                play_selected = !play_selected; 
                audio_play(play_selected, &mix);
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
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255); 
        SDL_RenderClear(renderer);

        // Render Frame

        // Play Button
        button.rect.x = 93; button.rect.y = 50; button.selected = &play_selected;
        if (add_button(&button, &mouse, renderer, &event))
        {
            if (play_selected) { mix.playhead = 0; }
            audio_play(play_selected, &mix);
        }
		add_text("Play", button.center.x, 40, font, renderer);

        // BPM Variables
        static int bpm = 60;
        static char bpm_str[MAX_CHARS];

        // BPM Decrement Button
        button.rect.x += 68; button.selected = NULL;
        if (add_button(&button, &mouse, renderer, &event))
        {
            bpm--;
        }
        add_text("-", button.center.x, button.center.y, font, renderer);

        // BPM Display Name
        add_text("BPM", button.center.x, 40, font, renderer);

        // BPM Increment Button
        button.rect.x += 34; 
        if (add_button(&button, &mouse, renderer, &event))
        {
            bpm++;
        }
        add_text("+", button.center.x, button.center.y, font, renderer);

        // BPM Display Value
        snprintf(bpm_str, MAX_CHARS, "%d", bpm);
        add_text(bpm_str, button.center.x, 40, font, renderer);

        // Bars Variables
        static int bars = 0;
        static char bars_str[MAX_CHARS];

        // Bars Decrement Button
        button.rect.x += 68; 
        if (add_button(&button, &mouse, renderer, &event))
        {
            if (bars > 0) { bars--; }
        }
        add_text("-", button.center.x, button.center.y, font, renderer);

        // Bars Display Name
        add_text("Bars", button.center.x, 40, font, renderer);

        // Bars Increment Button
        button.rect.x += 34; 
        if (add_button(&button, &mouse, renderer, &event))
        {
            if (bars < 16) { bars++; }
        }
        add_text("+", button.center.x, button.center.y, font, renderer);

        // Bars Display Value
        snprintf(bars_str, MAX_CHARS, "%d", bars);
        add_text(bars_str, button.center.x, 40, font, renderer);

        // Instrument Pads
        static float track_volume[NUM_TRACKS] = {MAX_VOLUME, MAX_VOLUME, MAX_VOLUME};


		// TODO: make this faster/better?

		// add pads one row at a time
        button.rect.x = 93; button.rect.y = 100;
		int track_count = 0;
		for (int i = 0; i < NUM_TRACKS*NUM_STEPS; i++)
		{
			button.selected = &pad_selected[i];
			add_button(&button, &mouse, renderer, &event);
			button.rect.x += button.rect.w + 10;

			// add track titles and move to next track
			if ((i+1) % NUM_STEPS == 0)
			{
				button.rect.x = 93;
				add_text(track_names[track_count], 63, button.center.y, font, renderer);
				button.rect.y += button.rect.h + 10;
				track_count++;
			}
		}

		// add slider for each track
        slider.rect.x = 637; slider.rect.y = 100;
        for (int i = 0; i < NUM_TRACKS; i++)
        {
            slider.rect.x += slider.rect.w + 2;
            slider.fill.h = (int)(track_volume[i] / MAX_VOLUME * slider.rect.h);
			slider.button.selected = &slider_selected[i];
            add_slider(&slider, &mouse, renderer, &event);

            if (slider_selected[i])
            {
                if (mouse.y > slider.rect.y && mouse.y < slider.rect.y + slider.rect.h)
                {
                    float new_height = (float)((slider.rect.y + slider.rect.h) - mouse.y);
                    float percent_full = new_height / (float)slider.rect.h;
                    track_volume[i] = percent_full * MAX_VOLUME;
                }
                else if (mouse.y > slider.rect.y + slider.rect.h)
                {
                    track_volume[i] = 0.0;
                }
                else if (mouse.y < slider.rect.y)
                {
                    track_volume[i] = MAX_VOLUME;
                }
            }
        }

		// Fill mix buffer with current pad sequence
		for (int i = 0; i < NUM_TRACKS; i++)
		{
			audio_fill_mix(&tracks[i], &mix, &bpm, &pad_selected[NUM_STEPS*i], track_volume[i]);
		}

        // Save Button
        button.rect.x = 603; button.rect.y = 215; button.selected = NULL;
        if (add_button(&button, &mouse, renderer, &event))
        {
			static int track_num = 1;
        	char file_name[MAX_CHARS];
        	snprintf(file_name, MAX_CHARS, "%s%d%s", "track-", track_num, ".wav");

            if (!audio_export_wave(file_name, bars, &mix))
            {
                fprintf(stderr, "ERROR in %s, line %d: failed to export pattern.\n", __FILE__, __LINE__);
            }
            else { track_num++; }
        }
        add_text("Save", 573, button.center.y, font, renderer);
            
        // Present Frame
        SDL_RenderPresent(renderer);
    }

    // Clean Up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_CloseAudioDevice(mix.device_id);

	for (int i = 0; i < NUM_TRACKS; i++) { SDL_FreeWAV((Uint8*)tracks[i].buffer); }
    free(mix.buffer);

    TTF_CloseFont(font);

    TTF_Quit();
    SDL_Quit();

    return 0;
}
