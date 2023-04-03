#include <stdio.h>
#include <assert.h>

#include "fours.h"
#include "mixer.h"

#define SCREEN_WIDTH (720)
#define SCREEN_HEIGHT (295)

#define BUTTON_DEFAULT_WIDTH (24)
#define BUTTON_DEFAULT_HEIGHT (24)

#define SLIDER_DEFAULT_WIDTH (9)
#define SLIDER_DEFAULT_HEIGHT (92)

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

// Private methods 
void _update_center(Position* center, SDL_Rect* rect);
bool _over_button(SDL_Rect* rect);

bool graphics_init()
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

void graphics_clear_screen(SDL_Color color)
{
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); 
	SDL_RenderClear(renderer);
}

void graphics_display()
{
	SDL_RenderPresent(renderer);
}

void graphics_close()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    TTF_CloseFont(font);
    TTF_Quit();
}

// BUTTON DATA/METHODS *******************************************************

Button button_new_default(int x, int y, bool* selected)
{
    Button button;

    button.rect.x = x;
    button.rect.y = y;

    button.rect.w = BUTTON_DEFAULT_WIDTH;
    button.rect.h = BUTTON_DEFAULT_HEIGHT;

    _update_center(&button.center, &button.rect); 

    button.selected = selected;

    return button;
}

bool add_button(Button* button)
{
    assert(button != NULL);

    _update_center(&button->center, &button->rect);

    SDL_Color color = UI_DEFAULT_BACKGROUND_COLOR;
    bool clicked_button = false;

    if (_over_button(&button->rect))
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
    SDL_RenderFillRect(renderer, &button->rect);

    color = UI_DEFAULT_OUTLINE_COLOR;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); 
    SDL_RenderDrawRect(renderer, &button->rect);

    return clicked_button;
}

// SLIDER METHODS ************************************************************

Slider slider_new_default(int x, int y)
{
    Slider slider;

    slider.rect.x = x;
    slider.rect.y = y;
    slider.rect.w = SLIDER_DEFAULT_WIDTH; 
    slider.rect.h = SLIDER_DEFAULT_HEIGHT; 

    slider.fill.x = x;
    slider.fill.y = y;
    slider.fill.w = SLIDER_DEFAULT_WIDTH; 
    slider.fill.h = SLIDER_DEFAULT_HEIGHT; 

    Button button = button_new_default(x, y, NULL);
	button.rect.x = x;
	button.rect.y = y;
    button.rect.w = slider.rect.w;
    button.rect.h = slider.rect.w;

    slider.button = button;

    return slider;
}

float add_slider(Slider* slider)
{
    assert(slider != NULL);

    SDL_Color color = UI_DEFAULT_BACKGROUND_COLOR;

	slider->fill.x = slider->rect.x; 
	slider->fill.y = slider->rect.y + slider->rect.h - slider->fill.h; 
	slider->button.rect.x = slider->rect.x;
	slider->button.rect.y = slider->fill.y; 
	
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); 
    SDL_RenderFillRect(renderer, &slider->rect);
    color = UI_DEFAULT_OUTLINE_COLOR;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); 
    SDL_RenderDrawRect(renderer, &slider->rect);

    color = UI_DEFAULT_SELECTED_COLOR;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); 
    SDL_RenderFillRect(renderer, &slider->fill);

    add_button(&slider->button);

	static float percent_full = 0.0;
	if (*slider->button.selected)
	{
		if (state.mouse.y > slider->rect.y && state.mouse.y < slider->rect.y + slider->rect.h)
		{
			float new_height = (float)((slider->rect.y + slider->rect.h) - state.mouse.y);
		    percent_full = new_height / (float)slider->rect.h;
		}
		else if (state.mouse.y > slider->rect.y + slider->rect.h)
		{
			percent_full = 0.0;
		}
        else if (state.mouse.y < slider->rect.y)
		{
			percent_full = 1.0;
		}
	}
	return percent_full;
}

// TEXT METHODS **************************************************************

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

// PRIVATE METHODS ***********************************************************

void _update_center(Position* center, SDL_Rect* rect)
{
    assert(rect != NULL);

	center->x = rect->x + (rect->w / 2);
	center->y = rect->y + (rect->h / 2);
}

bool _over_button(SDL_Rect* rect)
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

typedef struct
{
    SDL_AudioSpec spec;
    SDL_AudioDeviceID device_id;
} SDL_Audio;

SDL_Audio audio_out = {0};

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
        if (state.mix.playhead >= state.mix.length) 
        { 
            state.mix.playhead = 0; 
        }
        stream16[i] = (int16_t)state.mix.buffer[state.mix.playhead];
        state.mix.playhead += 1;

		// stream16[i] = (int16_t)(5000 * sin(2.0 * M_PI * 800.0 * (float)i / 44100.0));
    }
}

bool audio_init()
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

void audio_toggle()
{
    if (state.play_selected) { SDL_PauseAudioDevice(audio_out.device_id, 0); }

    else 
	{ 
		SDL_PauseAudioDevice(audio_out.device_id, 1); 
		state.mix.playhead = 0;
	}
}

void audio_close()
{
    SDL_CloseAudioDevice(audio_out.device_id);
    free(state.mix.buffer);
}

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

	fours_init(&state);

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
                audio_toggle();
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
		graphics_clear_screen(color);

		// Render Frame
		fours_render_graphics(&state); 

		// Present Frame
		graphics_display();

		SDL_LockAudioDevice(audio_out.device_id);
		fours_render_audio(&state);
		SDL_UnlockAudioDevice(audio_out.device_id);
	}

	// Clean Up
	audio_close();
	fours_close();
	graphics_close();

	return 0;
}
