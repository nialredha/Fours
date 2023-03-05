#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>
#include <math.h>
#include <SDL.h>

#include "position.h"

#define BUTTON_DEFAULT_WIDTH (24)
#define BUTTON_DEFAULT_HEIGHT (24)

#define SLIDER_DEFAULT_WIDTH (9)
#define SLIDER_DEFAULT_HEIGHT (92)

SDL_Color BUTTON_DEFAULT_OUTLINE = {255, 255, 255, 255};
SDL_Color BUTTON_DEFAULT_HOVER = {74, 74, 56, 255};
SDL_Color BUTTON_DEFAULT_SELECTED = {150, 150, 150, 255};
SDL_Color BUTTON_DEFAULT_BACKGROUND = {37, 37, 38, 255};

typedef struct
{
    int x, y; 
    int w, h;
    int cx, cy;
    Reference_Point ref;
} Rect;

typedef struct 
{
    Rect rect;
    SDL_Color background_color, outline_color, hover_color, selected_color;
    bool* selected;
} Button;

typedef struct 
{
    Rect rect;
    int fill_width, fill_height;
    SDL_Color fill_color, background_color, outline_color;
    Button button;
} Slider;

void button_update_center(Rect* rect)
{
    assert(rect != NULL);

    switch (rect->ref.x)
    {
        case LEFT:
            rect->cx = rect->x + (rect->w / 2);
            break;
        case RIGHT:
            rect->cx = rect->x - (rect->w / 2);
            break;
        case CENTER:
            rect->cx = rect->x;
            break;
    }
    switch (rect->ref.y)
    {
        case TOP:
            rect->cy = rect->y + (rect->h / 2);
            break;
        case BOTTOM:
            rect->cy = rect->y - (rect->h / 2);
            break;
        case CENTER:
            rect->cy = rect->y;
            break;
    }
}

Button button_new_default(int x, int y, Reference ref_x, Reference ref_y, bool* selected)
{
    Button button;

    button.rect.x = x;
    button.rect.y = y;

    button.rect.w = BUTTON_DEFAULT_WIDTH;
    button.rect.h = BUTTON_DEFAULT_HEIGHT;

    button.rect.ref.x = ref_x;
    button.rect.ref.y = ref_y;

    button_update_center(&button.rect); 

    button.background_color = BUTTON_DEFAULT_BACKGROUND;
    button.outline_color = BUTTON_DEFAULT_OUTLINE;
    button.hover_color = BUTTON_DEFAULT_HOVER;
    button.selected_color = BUTTON_DEFAULT_SELECTED;

    button.selected = selected;

    return button;
}

Slider slider_new_default(int x, int y, Reference ref_x, Reference ref_y)
{
    Slider slider;

    slider.rect.x = x;
    slider.rect.y = y;

    slider.rect.w = SLIDER_DEFAULT_WIDTH; 
    slider.rect.h = SLIDER_DEFAULT_HEIGHT; 

    slider.rect.ref.x = ref_x;
    slider.rect.ref.y = ref_y;

    button_update_center(&slider.rect);

    slider.fill_width = SLIDER_DEFAULT_WIDTH; 
    slider.fill_height = SLIDER_DEFAULT_HEIGHT; 

    slider.background_color = BUTTON_DEFAULT_BACKGROUND;
    slider.outline_color = BUTTON_DEFAULT_OUTLINE;
    slider.fill_color = BUTTON_DEFAULT_SELECTED;

    Button button = button_new_default(x, y, ref_x, TOP, NULL);
    button.rect.w = slider.rect.w;
    button.rect.h = slider.rect.w;

    slider.button = button;

    return slider;
}

bool over_button(SDL_Rect* rect, Position* mouse)
{
    assert(rect != NULL);
    assert(mouse != NULL);

    if (mouse->x > rect->x && mouse->x < rect->x + rect->w)
    {
        if (mouse->y > rect->y && mouse->y < rect->y + rect->h) 
        { 
            return true; 
        }
    }
    return false;
}

void transform_reference_frame(Rect* rect, SDL_Rect* sdl_rect)
{

    if (rect->ref.x == CENTER) { sdl_rect->x -= (int)(sdl_rect->w / 2); }
    else if (rect->ref.x == RIGHT) { sdl_rect->x -= sdl_rect->w; }

    if (rect->ref.y == CENTER) { sdl_rect->y -= (int)(sdl_rect->h / 2); }
    else if (rect->ref.y == BOTTOM) { sdl_rect->y -= sdl_rect->h; }
}

bool add_button(Button* button, Position* mouse, SDL_Renderer* rend, SDL_Event* event)
{
    assert(button != NULL);
    assert(mouse != NULL);
    assert(rend != NULL);
    assert(event != NULL);

    button_update_center(&button->rect);

    SDL_Rect rect = {button->rect.x, button->rect.y, button->rect.w, button->rect.h};
    SDL_Color color = button->background_color;
    bool clicked_button = false;

    if (button->selected != NULL && *button->selected) 
    { 
        color = button->selected_color;
    }

    if (over_button(&rect, mouse))
    {
        color = button->hover_color;

        if (event->type == SDL_MOUSEBUTTONDOWN) 
        { 
            clicked_button = true; 
            if (button->selected != NULL) { *button->selected = !*button->selected; }
        }
    }
    
    transform_reference_frame(&button->rect, &rect);

    SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, color.a); 
    SDL_RenderFillRect(rend, &rect);

    color = button->outline_color;
    SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, color.a); 
    SDL_RenderDrawRect(rend, &rect);

    return clicked_button;
}

bool add_slider(Slider* slider, Position* mouse, SDL_Renderer* rend, SDL_Event* event)
{
    assert(slider != NULL);
    assert(mouse != NULL);
    assert(rend != NULL);
    assert(event != NULL);

    button_update_center(&slider->rect);

    SDL_Color color = slider->background_color;
    SDL_Rect slide = {slider->rect.x, slider->rect.y, slider->rect.w, slider->rect.h};
    SDL_Rect fill = {slider->rect.x, slider->rect.y, slider->rect.w, slider->fill_height};

    transform_reference_frame(&slider->rect, &slide);
    transform_reference_frame(&slider->rect, &fill);

    SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, color.a); 
    SDL_RenderFillRect(rend, &slide);
    color = slider->outline_color;
    SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, color.a); 
    SDL_RenderDrawRect(rend, &slide);

    color = slider->fill_color;
    SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, color.a); 
    SDL_RenderFillRect(rend, &fill);

    return add_button(&slider->button, mouse, rend, event);
}

#endif // BUTTON_H
