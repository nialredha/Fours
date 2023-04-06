#ifndef SDL_FOURS_H
#define SDL_FOURS_H

#include <stdbool.h>

#include <SDL.h>
#include <SDL_ttf.h>

// TODO: Move UI definitions to fours.h. sdl_fours.c will implement them for SDL
// *****************************************************************************
typedef struct 
{
    int x, y; 
} Position;

typedef struct 
{
    SDL_Rect rect;
	Position center;
    bool* selected;
} Button;

typedef struct 
{
    SDL_Rect rect;
    SDL_Rect fill;
    Button button;
	bool* drag;
} Slider;

// UI
Button button_new_default(int x, int y, bool* selected);
bool add_button(Button* button);

Slider slider_new_default(int x, int y);
float add_slider(Slider* slider);

bool add_text(char* text, int x, int y);
// *****************************************************************************

// TODO: remove SDL Graphics and Audio definitions. These will only exit in sdl_fours.c
// SDL Graphics
bool graphics_init();
void graphics_clear_screen(SDL_Color color);
void graphics_display();
void graphics_close();

// SDL Audio
void audio_callback(void* userdata, Uint8* stream, int length);
bool audio_init();
void audio_toggle();
void audio_close();

#endif // SDL_FOURS_H 
