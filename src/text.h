#ifndef TEXT_H
#define TEXT_H

#include <stdbool.h>
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
    Position_Ref x_ref, y_ref;
    char* value;
    Font font;
} Text;

Text text_new(int x, int y, Position_Ref x_ref, Position_Ref y_ref, char* val, char* file, int size)
{
    Text text; 

    text.x = x;
    text.y = y;
    text.x_ref = x_ref;
    text.y_ref = y_ref;
    text.value = val;
    text.font.path = file;
    text.font.size = size;

    return text;
}

void text_update(int x, int y, Position_Ref x_ref, Position_Ref y_ref, char* value, Text* text)
{
    text->x = x;
    text->y = y;
    text->x_ref = x_ref;
    text->y_ref = y_ref;
    text->value = value;
}

bool add_text(Text* text, SDL_Renderer* rend)
{
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
    else 
    {
        rect.w = 0;
        rect.h = 0;
    }

    if (text->x_ref == CENTER) { rect.x -= (int)(rect.w / 2); }
    else if (text->x_ref == RIGHT) { rect.x -= rect.w; }
    if (text->y_ref == CENTER) { rect.y -= (int)(rect.h / 2); }
    else if (text->y_ref == BOTTOM) { rect.y -= rect.h; }

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
