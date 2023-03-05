#ifndef TEXT_H
#define TEXT_H

#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <SDL_ttf.h>

#include "position.h"

typedef struct
{
    char* path;
    int size;
} Font;

typedef struct
{
    int x, y;
    Reference_Point ref;
    char* value;
    Font font;
} Text;

Text text_new(int x, int y, Reference ref_x, Reference ref_y, char* val, char* file, int size)
{
    Text text; 

    text.x = x;
    text.y = y;
    text.ref.x = ref_x;
    text.ref.y = ref_y;
    text.value = val;
    text.font.path = file;
    text.font.size = size;

    return text;
}

bool add_text(Text* text, SDL_Renderer* rend)
{
    assert(text != NULL);
    assert(rend != NULL);

    TTF_Font* font = TTF_OpenFont(text->font.path, text->font.size);
    if (!font)
    {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
        return false;
    }

    SDL_Color color = {255, 255, 255, 255};
    SDL_Rect rect = {text->x, text->y, 0, 0};
    SDL_Surface* surface = TTF_RenderText_Blended(font, text->value, color);

    if (surface != NULL) 
    {
        rect.w = surface->w;
        rect.h = surface->h;
    }

    if (text->ref.x == CENTER) { rect.x -= (int)(rect.w / 2); }
    else if (text->ref.x == RIGHT) { rect.x -= rect.w; }

    if (text->ref.y == CENTER) { rect.y -= (int)(rect.h / 2); }
    else if (text->ref.y == BOTTOM) { rect.y -= rect.h; }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(rend, surface);
    SDL_FreeSurface(surface);

    if (texture == NULL)
    {
        fprintf(stderr, "ERROR in %s, line %d: Couldn't create texture path\n", __FILE__,  __LINE__);
        return false;
    }

    SDL_RenderCopy(rend, texture, NULL, &rect);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);

    return true;
}

#endif // TEXT_H
