#include <stdbool.h>

#include "SDL.h"
#include "math.h"

#define BUTTON_WIDTH (25)
#define BUTTON_HEIGHT (25)

#define MAX(X, Y) (X < Y ? X : Y)

SDL_Color button_outline = {255, 255, 255, 255};
SDL_Color button_hover = {74, 74, 56, 255};
SDL_Color button_selected = {139, 0, 0, 255};
SDL_Color button_background = {37, 37, 38, 255};

bool over_button(SDL_Rect* rect, int mouse_x, int mouse_y)
{
    if (mouse_x > rect->x && mouse_x < rect->x + rect->w)
    {
        if (mouse_y > rect->y && mouse_y < rect->y + rect->h) 
        { 
            return true; 
        }
    }
    return false;
}

bool add_button(int x, int y, int mouse_x, int mouse_y, bool* selected, SDL_Renderer* rend, SDL_Event* event)
{
    SDL_Rect rect = {x, y, BUTTON_WIDTH, BUTTON_HEIGHT};
    SDL_Color color = button_background;
    bool clicked_button = false;

    if (selected != NULL && *selected) 
    { 
        color = button_selected;
        SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, color.a); 
        SDL_RenderFillRect(rend, &rect);
    }

    if (over_button(&rect, mouse_x, mouse_y))
    {
        SDL_SetRenderDrawColor(rend, MAX(color.r*2, 255), MAX(color.g*2, 255), MAX(color.b*2, 255), color.a); 
        SDL_RenderFillRect(rend, &rect);

        if (event->type == SDL_MOUSEBUTTONDOWN) 
        { 
            clicked_button = true; 
            if (selected != NULL) { *selected = !*selected; }
        }
    }

    color = button_outline;
    SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, color.a); 
    SDL_RenderDrawRect(rend, &rect);

    return clicked_button;
}

