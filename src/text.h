#include <stdbool.h>

#include "SDL.h"
#include "SDL_ttf.h"

void add_text(int x, int y, char* text, TTF_Font* font, SDL_Renderer* rend)
{
    SDL_Color color = {255, 255, 255, 255};
    SDL_Rect rect = {x, y, 0, 0};
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);

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

    SDL_Texture* texture = SDL_CreateTextureFromSurface(rend, surface);
    SDL_FreeSurface(surface);

    if (texture == NULL)
    {
        fprintf(stderr, "ERROR in %s, line %d: Couldn't create texture path\n", __FILE__,  __LINE__);
        exit(1);
    }

    SDL_RenderCopy(rend, texture, NULL, &rect);
    SDL_DestroyTexture(texture);
}
