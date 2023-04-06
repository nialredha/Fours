#include "fours.c"

#include <stdio.h>
#include <assert.h>

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

#define SCREEN_WIDTH (720)
#define SCREEN_HEIGHT (295)

// GLOBAL VARIABLES
const SDL_Color UI_DEFAULT_OUTLINE_COLOR = {255, 255, 255, 255};
const SDL_Color UI_DEFAULT_HOVER_COLOR = {74, 74, 56, 255};
const SDL_Color UI_DEFAULT_SELECTED_COLOR = {150, 150, 150, 255};
const SDL_Color UI_DEFAULT_BACKGROUND_COLOR = {37, 37, 38, 255};

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;

SDL_Event event;
Fours_State state;

typedef struct
{
    SDL_AudioSpec spec;
    SDL_AudioDeviceID device_id;
} SDL_Audio;

SDL_Audio audio_out = {0};

void update_center(Position* center, SDL_Rect* rect)
{
    assert(rect != NULL);

	center->x = rect->x + (rect->w / 2);
	center->y = rect->y + (rect->h / 2);
}

bool over_button(SDL_Rect* rect)
{
    assert(rect != NULL);

    if (state.mouse.x > rect->x && state.mouse.x < rect->x + rect->w)
    {
        if (state.mouse.y > rect->y && state.mouse.y < rect->y + rect->h) 
        { 
            return true; 
        }
    }
    return false;
}

bool add_button(Button* button)
{
    assert(button != NULL);

	SDL_Rect rect = {0};
	rect.w = button->width; 
	rect.h = button->height; 
	rect.x = button->origin.x;
	rect.y = button->origin.y;

    update_center(&button->center, &rect);

    SDL_Color color = UI_DEFAULT_BACKGROUND_COLOR;
    bool clicked_button = false;

    if (over_button(&rect))
    {
        color = UI_DEFAULT_HOVER_COLOR;

        if (event.type == SDL_MOUSEBUTTONDOWN) 
        { 
            clicked_button = true; 
			if (button->selected) { *button->selected = !*button->selected; }
        }
    }
	else if (button->selected && *button->selected)
	{
		color = UI_DEFAULT_SELECTED_COLOR; 
	}
		
    
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); 
    SDL_RenderFillRect(renderer, &rect);

    color = UI_DEFAULT_OUTLINE_COLOR;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); 
    SDL_RenderDrawRect(renderer, &rect);

    return clicked_button;
}

float add_slider(Slider* slider)
{
    assert(slider != NULL);

    SDL_Color color = UI_DEFAULT_BACKGROUND_COLOR;

	SDL_Rect fill = {0};
	fill.x = slider->origin.x; 
	fill.y = slider->origin.y + slider->fill_height; 
	fill.w = slider->width;
	fill.h = slider->fill_height;

	slider->button.origin.x = slider->origin.x;
	slider->button.origin.y = fill.y; 

	SDL_Rect slider_box = {0};
	slider_box.x = slider->origin.x;
	slider_box.y = slider->origin.y;
	slider_box.w = slider->width;
	slider_box.h = slider->height;
	
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); 
    SDL_RenderFillRect(renderer, &slider_box);
    color = UI_DEFAULT_OUTLINE_COLOR;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); 
    SDL_RenderDrawRect(renderer, &slider_box);

    color = UI_DEFAULT_SELECTED_COLOR;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); 
    SDL_RenderFillRect(renderer, &fill);

    add_button(&slider->button);

	static float percent_full = 0.0;
	if (*slider->button.selected)
	{
		if (state.mouse.y > slider->origin.y && state.mouse.y < slider->origin.y + slider->height)
		{
			float new_height = (float)((slider->origin.y + slider->height) - state.mouse.y);
		    percent_full = new_height / (float)slider->height;
		}
		else if (state.mouse.y > slider->origin.y + slider->height)
		{
			percent_full = 0.0;
		}
        else if (state.mouse.y < slider->origin.y)
		{
			percent_full = 1.0;
		}
	}
	return percent_full;
}

bool add_text(char* text, int x, int y)
{
    assert(text != NULL);

	SDL_Rect rect = { x, y, 0, 0 };
    SDL_Color color = UI_DEFAULT_OUTLINE_COLOR;
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);

    if (surface != NULL) 
    {
        rect.w = surface->w;
        rect.h = surface->h;

		rect.x -= rect.w/2; 
		rect.y -= rect.h/2; 
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (texture == NULL)
    {
        fprintf(stderr, "ERROR in %s, line %d: Couldn't create texture path\n", __FILE__,  __LINE__);
        return false;
    }

    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    return true;
}

void toggle_audio()
{
    if (state.play_selected) { SDL_PauseAudioDevice(audio_out.device_id, 0); }

    else 
	{ 
		SDL_PauseAudioDevice(audio_out.device_id, 1); 
		state.readhead = 0;
	}
}

void audio_callback(void* userdata, Uint8* stream, int length)
{
    int16_t* stream16 = (int16_t*)stream;
    int length16 = length / 2; 
    userdata = NULL; // not sure how to make use of this
    
    // clear buffer to 0
    for (int i = 0; i < length16; i++)
    {
        stream16[i] = 0;
    }

    // fill buffer with mixed audio
    for (int i = 0; i < length16; i++)
    {
        if (state.readhead >= state.audio_length) 
        { 
            state.readhead = 0; 
        }
        stream16[i] = (int16_t)state.audio_buffer[state.readhead];
        state.readhead += 1;
    }
}

bool init_audio()
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
		return false;
	}

	// TODO: not sure how these values should be determined
	audio_out.spec.callback = audio_callback;
	audio_out.spec.freq = 44100;
	audio_out.spec.format = AUDIO_S16;
	audio_out.spec.channels = 1;
	audio_out.spec.samples = 1024;

	SDL_AudioSpec output_spec;
    audio_out.device_id = SDL_OpenAudioDevice(NULL, 0, &audio_out.spec, &output_spec, 0);

    if (audio_out.device_id == 0)
    {
        fprintf(stderr, "SDL_OpenAudioDevice Error: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

void close_audio()
{
    SDL_CloseAudioDevice(audio_out.device_id);
    free(state.audio_buffer);
}

bool init_graphics()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
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

void clear_screen(SDL_Color color)
{
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); 
	SDL_RenderClear(renderer);
}

void close_graphics()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    TTF_CloseFont(font);
    TTF_Quit();
}


int main(int argc, char** argv)
{
    if (argc > 1)
    {
        fprintf(stderr, "WARNING: input '%s', but program doesn't except input arguments.\n", argv[1]);
    }

    // SDL Audio Visual Initialization
    if (!init_graphics() | !init_audio()) 
    { 
        fprintf(stderr, "ERROR in %s, line %d: failed to initialize.\n", __FILE__, __LINE__);
        exit(1); 
    }

	if (!fours_init(&state))
	{
        fprintf(stderr, "ERROR in %s, line %d: failed to initialize.\n", __FILE__, __LINE__);
        exit(1); 
	}

    // Main Loop
    bool quit = false;
    while (!quit)
    {
        SDL_GetMouseState(&state.mouse.x, &state.mouse.y);

        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) 
        { 
            quit = true; 
        }
        else if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_SPACE) 
            { 
                state.play_selected = !state.play_selected; 
                toggle_audio();
            }
        }
        else if (event.type == SDL_MOUSEBUTTONUP)
        {
            for (int i = 0; i < NUM_SAMPLES; i++)
            {
                if (state.slider_selected[i])
                {
                    state.slider_selected[i] = false;
                }
            }
        }

        // Clear screen to gray
		SDL_Color color = {30, 30, 30, 255};
		clear_screen(color);

		// Render Frame
		fours_render_graphics(&state); 

		// Present Frame
		SDL_RenderPresent(renderer);

		SDL_LockAudioDevice(audio_out.device_id);
		fours_render_audio(&state);
		SDL_UnlockAudioDevice(audio_out.device_id);
	}

	// Clean Up
	close_audio();
	close_graphics();

	fours_close();

	return 0;
}
