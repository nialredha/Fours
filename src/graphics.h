#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <SDL.h>
#include <SDL_ttf.h>

#define SCREEN_WIDTH (720)
#define SCREEN_HEIGHT (295)

#define BUTTON_DEFAULT_WIDTH (24)
#define BUTTON_DEFAULT_HEIGHT (24)

#define SLIDER_DEFAULT_WIDTH (9)
#define SLIDER_DEFAULT_HEIGHT (92)

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
bool add_slider(Slider* slider, Position* mouse, SDL_Event* event);

bool add_text(char* text, int x, int y);

#endif // GRAPHICS_H
