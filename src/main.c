#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "audio.h"
#include "button.h"
#include "text.h"

#define SCREEN_WIDTH (720)
#define SCREEN_HEIGHT (295)
#define NUM_INSTRUMENTS (3)

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;

SDL_Color white = {255, 255, 255, 255};
SDL_Color dark_red = {139, 0, 0, 255};
SDL_Color gray = {30, 30, 30, 255};

Mix16 mix = {0};
Track16 kick = {0};
Track16 hat = {0};
Track16 snare = {0};

typedef struct Position
{
    int x, y; 
} Position;

void audio_callback(void* userdata, Uint8* stream, int length)
{
    Uint16* stream16 = (Uint16*)stream;
    int length16 = length / BYTES_PER_SAMPLE; 
    userdata = NULL; // not sure how to make use of this
    
    // clear buffer to 0
    for (int i = 0; i < length16; i++)
    {
        stream16[i] = 0;
    }

    // fill buffer with mixed audio
    for (int i = 0; i < length16; i++)
    {
        if (mix.playhead >= mix.length) 
        { 
            mix.playhead = 0; 
        }
        stream16[i] = mix.buffer[mix.playhead];
        mix.playhead += 1;
    }
}

bool init_av()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }

    if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN, 
                                    &window, &renderer) < 0)
    {
        fprintf(stderr, "SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
        return false;
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

    return true;
}

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        fprintf(stderr, "Inputted %s, but this program does not except any arguments\n", argv[1]);
    }

    if (!init_av()) { exit(1); }

    SDL_Event event;
    Position mouse;

    bool play_selected = false;
    bool pad_selected[NUM_PADS] = {false};

    int bpm = 60;
    char bpm_str[10];

    mix = audio_new_mix(NUM_INSTRUMENTS);
    kick = audio_new_track("../assets/kick.wav");
    hat = audio_new_track("../assets/hihat.wav");
    snare = audio_new_track("../assets/snare.wav");

    if(!audio_load_track(&kick, &mix))  { exit(1); }; 
    if(!audio_load_track(&hat, &mix))   { exit(1); }; 
    if(!audio_load_track(&snare, &mix)) { exit(1); }; 

    mix.spec.callback = audio_callback;
    if(!audio_open(&mix)) { exit(1); }; 

    bool quit = false;
    while (!quit)
    {
        SDL_GetMouseState(&mouse.x, &mouse.y);

        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) 
        { 
            audio_play(false, &mix);
            quit = true; 
        }

        // clear the screen to gray
        SDL_SetRenderDrawColor(renderer, gray.r, gray.g, gray.b, gray.a); 
        SDL_RenderClear(renderer);

        // Render Frame

        // Play Button
        if (add_button(85, 50, mouse.x, mouse.y, &play_selected, renderer, &event))
        {
            if (play_selected) { mix.playhead = 0; }
            audio_play(play_selected, &mix);
        }
        add_text(85, 26, "Play", font, renderer);

        // BPM UI
        if (add_button(155, 50, mouse.x, mouse.y, NULL, renderer, &event))
        {
            bpm--;
        }
        add_text(165, 53, "-", font, renderer);

        if (add_button(190, 50, mouse.x, mouse.y, NULL, renderer, &event))
        {
            bpm++;
        }
        add_text(198, 53, "+", font, renderer);

        add_text(155, 26, "Bpm", font, renderer);
        sprintf(bpm_str, "%d", bpm);
        add_text(195, 26, bpm_str, font, renderer);
        
        // Instrument Pads
        int pad_x= 85, pad_y = 100, id = 0;
        char* instruments[3] = {"Kick", "HiHat", "Snare"};
        for (int i = 0; i < NUM_INSTRUMENTS; i++)
        {
            for (int j = 0; j < NUM_STEPS; j++)
            {
                id = (NUM_STEPS*i) + j;
                add_button(pad_x, pad_y, mouse.x, mouse.y, &pad_selected[id], renderer, &event);
                pad_x += BUTTON_WIDTH + 10;
            }
            add_text(35, pad_y + 3, instruments[i], font, renderer);

            pad_x = 85;
            pad_y += BUTTON_HEIGHT + 10;
        }

        if (!audio_fill_mix(&kick, &mix, bpm, &pad_selected[0]))    { exit(1); }
        if (!audio_fill_mix(&hat, &mix, bpm, &pad_selected[16]))    { exit(1); }
        if (!audio_fill_mix(&snare, &mix, bpm, &pad_selected[32]))  { exit(1); }

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

    TTF_CloseFont(font);

    SDL_CloseAudioDevice(mix.device_id);
    SDL_FreeWAV((Uint8*)kick.buffer);
    SDL_FreeWAV((Uint8*)mix.buffer);

    TTF_Quit();
    SDL_Quit();

    return 0;
}
