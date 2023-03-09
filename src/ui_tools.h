#ifndef UI_TOOLS_H
#define UI_TOOLS_H

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <SDL.h>
#include <SDL_ttf.h>

#define BUTTON_DEFAULT_WIDTH (24)
#define BUTTON_DEFAULT_HEIGHT (24)

#define SLIDER_DEFAULT_WIDTH (9)
#define SLIDER_DEFAULT_HEIGHT (92)

const SDL_Color UI_DEFAULT_OUTLINE_COLOR = {255, 255, 255, 255};
const SDL_Color UI_DEFAULT_HOVER_COLOR = {74, 74, 56, 255};
const SDL_Color UI_DEFAULT_SELECTED_COLOR = {150, 150, 150, 255};
const SDL_Color UI_DEFAULT_BACKGROUND_COLOR = {37, 37, 38, 255};

typedef struct 
{
    int x, y; 
} Position;

// Intended as private methods. Use with caution.
void _update_center(Position* center, SDL_Rect* rect);
bool _over_button(SDL_Rect* rect, Position* mouse);

// BUTTON DATA/METHODS *******************************************************

typedef struct 
{
    SDL_Rect rect;
	Position center;
    bool* selected;
} Button;

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

bool add_button(Button* button, Position* mouse, SDL_Renderer* rend, SDL_Event* event)
{
    assert(button != NULL);
    assert(mouse != NULL);
    assert(rend != NULL);
    assert(event != NULL);

    _update_center(&button->center, &button->rect);

    SDL_Color color = UI_DEFAULT_BACKGROUND_COLOR;
    bool clicked_button = false;

    if (_over_button(&button->rect, mouse))
    {
        color = UI_DEFAULT_HOVER_COLOR;

        if (event->type == SDL_MOUSEBUTTONDOWN) 
        { 
            clicked_button = true; 
			if (button->selected) { *button->selected = !*button->selected; }
        }
    }
	else if (button->selected && *button->selected)
	{
		color = UI_DEFAULT_SELECTED_COLOR; 
	}
		
    
    SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, color.a); 
    SDL_RenderFillRect(rend, &button->rect);

    color = UI_DEFAULT_OUTLINE_COLOR;
    SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, color.a); 
    SDL_RenderDrawRect(rend, &button->rect);

    return clicked_button;
}

// SLIDER METHODS ************************************************************

typedef struct 
{
    SDL_Rect rect;
    SDL_Rect fill;
    Button button;
} Slider;

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

bool add_slider(Slider* slider, Position* mouse, SDL_Renderer* rend, SDL_Event* event)
{
    assert(slider != NULL);
    assert(mouse != NULL);
    assert(rend != NULL);
    assert(event != NULL);

    SDL_Color color = UI_DEFAULT_BACKGROUND_COLOR;

	slider->fill.x = slider->rect.x; 
	slider->fill.y = slider->rect.y + slider->rect.h - slider->fill.h; 
	slider->button.rect.x = slider->rect.x;
	slider->button.rect.y = slider->fill.y; 
	
    SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, color.a); 
    SDL_RenderFillRect(rend, &slider->rect);
    color = UI_DEFAULT_OUTLINE_COLOR;
    SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, color.a); 
    SDL_RenderDrawRect(rend, &slider->rect);

    color = UI_DEFAULT_SELECTED_COLOR;
    SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, color.a); 
    SDL_RenderFillRect(rend, &slider->fill);

    return add_button(&slider->button, mouse, rend, event);
}

// TEXT METHODS **************************************************************

bool add_text(char* text, int x, int y, TTF_Font* font, SDL_Renderer* rend)
{
    assert(text != NULL);
    assert(rend != NULL);
	assert(font != NULL);

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

    SDL_Texture* texture = SDL_CreateTextureFromSurface(rend, surface);
    SDL_FreeSurface(surface);

    if (texture == NULL)
    {
        fprintf(stderr, "ERROR in %s, line %d: Couldn't create texture path\n", __FILE__,  __LINE__);
        return false;
    }

    SDL_RenderCopy(rend, texture, NULL, &rect);
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

bool _over_button(SDL_Rect* rect, Position* mouse)
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

#endif // UI_TOOLS_H
