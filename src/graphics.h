#ifndef GRAPHICS_H
#define GRAPHICS_H

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

bool graphics_init();
void graphics_clear_screen(SDL_Color color);
void graphics_display();
void graphics_close();

Button button_new_default(int x, int y, bool* selected);
bool add_button(Button* button, Position* mouse, SDL_Event* event);

Slider slider_new_default(int x, int y);
float add_slider(Slider* slider, Position* mouse, SDL_Event* event);

bool add_text(char* text, int x, int y);

#endif // GRAPHICS_H
