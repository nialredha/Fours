#ifndef SDL_FOURS_H
#define SDL_FOURS_H

#include <stdbool.h>

#include <SDL.h>
#include <SDL_ttf.h>

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

// SDL Graphics
bool graphics_init();
void graphics_clear_screen(SDL_Color color);
void graphics_display();
void graphics_close();

// UI
Button button_new_default(int x, int y, bool* selected);
bool add_button(Button* button);

Slider slider_new_default(int x, int y);
float add_slider(Slider* slider);

bool add_text(char* text, int x, int y);

// SDL Audio
void audio_callback(void* userdata, Uint8* stream, int length);
bool audio_init();
void audio_toggle();
void audio_close();

#endif // SDL_FOURS_H 
