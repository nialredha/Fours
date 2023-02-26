#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>
#include <math.h>

#include "SDL.h"
#include "position.h"

#define BUTTON_DEFAULT_WIDTH (24)
#define BUTTON_DEFAULT_HEIGHT (24)

SDL_Color BUTTON_DEFAULT_OUTLINE = {255, 255, 255, 255};
SDL_Color BUTTON_DEFAULT_HOVER = {74, 74, 56, 255};
SDL_Color BUTTON_DEFAULT_SELECTED = {150, 150, 150, 255};
SDL_Color BUTTON_DEFAULT_BACKGROUND = {37, 37, 38, 255};

typedef struct 
{
    int x, y;
    int width, height;
    int center_x, center_y;
    Position_Ref x_ref, y_ref;
    SDL_Color background_color, outline_color, hover_color, selected_color;
} Button;

void button_update_center(Button* button)
{
    switch (button->x_ref)
    {
        case LEFT:
            button->center_x = button->x + (button->width / 2);
            break;
        case RIGHT:
            button->center_x = button->x - (button->width / 2);
            break;
        case CENTER:
            button->center_x = button->x;
            break;
    }
    switch (button->y_ref)
    {
        case TOP:
            button->center_y = button->y + (button->height / 2);
            break;
        case BOTTOM:
            button->center_y = button->y - (button->height / 2);
            break;
        case CENTER:
            button->center_y = button->y;
            break;
    }
}
Button button_new_default(int x, int y, Position_Ref x_ref, Position_Ref y_ref)
{
    Button button;

    button.x = x;
    button.y = y;

    button.width = BUTTON_DEFAULT_WIDTH;
    button.height = BUTTON_DEFAULT_HEIGHT;

    button.x_ref = x_ref;
    button.y_ref = y_ref;

    button_update_center(&button); 

    button.background_color = BUTTON_DEFAULT_BACKGROUND;
    button.outline_color = BUTTON_DEFAULT_OUTLINE;
    button.hover_color = BUTTON_DEFAULT_HOVER;
    button.selected_color = BUTTON_DEFAULT_SELECTED;

    return button;
}

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

bool add_button(Button* button, bool* selected, int mouse_x, int mouse_y, SDL_Renderer* rend, SDL_Event* event)
{
    SDL_Rect rect = {button->x, button->y, button->width, button->height};
    SDL_Color color = button->background_color;
    bool clicked_button = false;

    if (selected != NULL && *selected) 
    { 
        color = button->selected_color;
    }

    if (over_button(&rect, mouse_x, mouse_y))
    {
        color = button->hover_color;

        if (event->type == SDL_MOUSEBUTTONDOWN) 
        { 
            clicked_button = true; 
            if (selected != NULL) { *selected = !*selected; }
        }
    }
    
    if (button->x_ref == CENTER) { rect.x -= (int)(rect.w / 2); }
    else if (button->x_ref == RIGHT) { rect.x -= rect.w; }
    if (button->y_ref == CENTER) { rect.y -= (int)(rect.h / 2); }
    else if (button->y_ref == BOTTOM) { rect.y -= rect.h; }

    SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, color.a); 
    SDL_RenderFillRect(rend, &rect);

    color = button->outline_color;
    SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, color.a); 
    SDL_RenderDrawRect(rend, &rect);

    return clicked_button;
}

#endif // BUTTON_H
