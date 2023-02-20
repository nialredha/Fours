/* Really Advanced WAVE Media Player Application (RAWMPA)*/ 

#include <stdio.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "button.h"
#include "text.h"

#define SCREEN_WIDTH (720)
#define SCREEN_HEIGHT (295)
#define NUM_INSTRUMENTS (3)
#define NUM_STEPS (16)
#define NUM_PADS (48)

SDL_Color white = {255, 255, 255, 255};
SDL_Color dark_red = {139, 0, 0, 255};

typedef struct Position
{
    int x, y; 
} Position;

int main(int argc, char** argv)
{
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    TTF_Font* font = NULL;

    if (argc > 1)
    {
        fprintf(stderr, "Inputted %s, but this program does not except any arguments\n", argv[1]);
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN, 
                                    &window, &renderer) < 0)
    {
        fprintf(stderr, "SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() < 0)
    {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        return false;
    }

    font = TTF_OpenFont("../assets/Roboto-Regular.ttf", 16);
    if (!font)
    {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
        return false;
    }

    SDL_ShowCursor(SDL_ENABLE);

    SDL_Color gray = {37, 37, 38, 255};
    SDL_Event event;
    Position mouse;

    bool play_selected = false;
    bool pad_selected[NUM_PADS] = {false};

    bool quit = false;
    while (!quit)
    {
        SDL_GetMouseState(&mouse.x, &mouse.y);

        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) { quit = true; }

        // clear the screen to gray
        SDL_SetRenderDrawColor(renderer, gray.r, gray.g, gray.b, gray.a); 
        SDL_RenderClear(renderer);

        // Render Frame

        // Play Button
        int play_x = 85, play_y = 50;
        add_text(35, play_y + 3, "Play", font, renderer);
        if (add_button(play_x, play_y, mouse.x, mouse.y, &play_selected, renderer, &event))
        {
            // TODO: Play Audio
            printf("play: %d\n", play_selected);
        }

        // TODO: Add BPM slider
        
        // Instruments Pads
        int pad_x= 85, pad_y = 100;
        char* instruments[3] = {"Kick", "HiHat", "Snare"};
        for (int i = 0; i < NUM_INSTRUMENTS; i++)
        {
            add_text(35, pad_y + 3, instruments[i], font, renderer);
            
            for (int j = 0; j < NUM_STEPS; j++)
            {
                int id = (NUM_STEPS*i) + j;
                if (add_button(pad_x, pad_y, mouse.x, mouse.y, &pad_selected[id], renderer, &event))
                {
                    printf("pad #%d: %d\n", id+1, pad_selected[id]);
                }
                pad_x += BUTTON_WIDTH + 10;
            }
            pad_x = 85;
            pad_y += BUTTON_HEIGHT + 10;
        }

        // Save Button
        int save_x = 610, save_y = 215; 
        add_text(565, save_y + 3, "Save", font, renderer);
        if (add_button(save_x, save_y, mouse.x, mouse.y, NULL, renderer, &event))
        {
            // TODO: save current audio buffer
            printf("selected save\n");
        }
            
        // Present Frame
        SDL_RenderPresent(renderer);
    }

    // Clean Up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
