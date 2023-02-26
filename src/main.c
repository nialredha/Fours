#include <stdio.h>
#include <stdbool.h>

#include <SDL.h>

#include "position.h"
#include "audio.h"
#include "button.h"
#include "text.h"

#define SCREEN_WIDTH (720)
#define SCREEN_HEIGHT (295)
#define NUM_INSTRUMENTS (3)

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Mix16 mix = {0};

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

    SDL_ShowCursor(SDL_ENABLE);

    return true;
}

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        fprintf(stderr, "Inputted %s, but this program does not except any arguments\n", argv[1]);
    }

    // SDL Audio Visual Initialization
    if (!init_av()) { exit(1); }

    // Text Setup
    char* roboto = "../assets/Roboto-Regular.ttf";
    Text text = text_new(85, 45, LEFT, BOTTOM, "", roboto, 16);

    // Button Setup
    Button button = button_new_default(0, 0, LEFT, TOP);
    bool play_selected = false;
    bool pad_selected[NUM_PADS] = {false};

    // Audio Setup
    Track16 kick = {0};
    Track16 hat = {0};
    Track16 snare = {0};

    mix = audio_new_mix(NUM_INSTRUMENTS);
    kick = audio_new_track("../assets/kick.wav");
    hat = audio_new_track("../assets/hihat.wav");
    snare = audio_new_track("../assets/snare.wav");

    if(!audio_load_track(&kick, &mix))  { exit(1); }; 
    if(!audio_load_track(&hat, &mix))   { exit(1); }; 
    if(!audio_load_track(&snare, &mix)) { exit(1); }; 

    mix.spec.callback = audio_callback;
    if(!audio_open(&mix)) { exit(1); }; 

    SDL_Event event;
    Position mouse;

    // Main Loop
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

        // Clear screen to gray
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255); 
        SDL_RenderClear(renderer);

        // Render Frame

        // Play Button
        button.x = 85; button.y = 50;
        button_update_center(&button);
        if (add_button(&button, &play_selected, mouse.x, mouse.y, renderer, &event))
        {
            if (play_selected) { mix.playhead = 0; }
            audio_play(play_selected, &mix);
        }
        text.x = 85; text.y = 45; text.x_ref = LEFT; text.y_ref = BOTTOM;
        text.value = "Play";
        add_text(&text, renderer);

        // BPM Variables
        static int bpm = 60;
        static char bpm_str[10];

        // BPM Decrement Button
        button.x = 155; button.y = 50;
        button_update_center(&button);
        if (add_button(&button, NULL, mouse.x, mouse.y, renderer, &event))
        {
            bpm--;
        }
        text.x = button.center_x; text.y = button.center_y; text.x_ref = CENTER; text.y_ref = CENTER;
        text.value = "-";
        add_text(&text, renderer);

        // BPM Increment Button
        button.x = 190; 
        button_update_center(&button);
        if (add_button(&button, NULL, mouse.x, mouse.y, renderer, &event))
        {
            bpm++;
        }
        text.x = button.center_x; text.y = button.center_y; 
        text.value = "+";
        add_text(&text, renderer);

        // BPM Display Value
        text.x = 155; text.y = 45; text.x_ref = LEFT; text.y_ref = BOTTOM;
        text.value = "BPM";
        add_text(&text, renderer);

        sprintf(bpm_str, "%d", bpm);
        text.x = 195; text.value = bpm_str;
        add_text(&text, renderer);
        
        // Instrument Pads
        int id = 0;
        button.x = 85; button.y = 100;
        char* instruments[3] = {"Kick", "HiHat", "Snare"};
        for (int i = 0; i < NUM_INSTRUMENTS; i++)
        {
            for (int j = 0; j < NUM_STEPS; j++)
            {
                id = (NUM_STEPS*i) + j;
                button_update_center(&button);
                add_button(&button, &pad_selected[id], mouse.x, mouse.y, renderer, &event);
                button.x += button.width + 10;
            }

            button.x = 85;

            text.x = button.x - 5; text.y = button.center_y; text.x_ref = RIGHT; text.y_ref = CENTER;
            text.value = instruments[i];
            add_text(&text, renderer);

            button.y += button.height + 10;
        }

        // Fill mix buffer with current pad sequence
        if (!audio_fill_mix(&kick, &mix, &bpm, &pad_selected[0]))    { exit(1); }
        if (!audio_fill_mix(&hat, &mix, &bpm, &pad_selected[16]))    { exit(1); }
        if (!audio_fill_mix(&snare, &mix, &bpm, &pad_selected[32]))  { exit(1); }

        // Save Button
        button.x = 610; button.y = 215;
        button_update_center(&button);
        if (add_button(&button, NULL, mouse.x, mouse.y, renderer, &event))
        {
            // TODO: write data to wave 
        }
        text.x = button.x - 5; text.y = button.center_y; 
        text.value = "Save";
        add_text(&text, renderer);
            
        // Present Frame
        SDL_RenderPresent(renderer);
    }

    // Clean Up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_CloseAudioDevice(mix.device_id);
    SDL_FreeWAV((Uint8*)kick.buffer);
    SDL_FreeWAV((Uint8*)mix.buffer);

    TTF_Quit();
    SDL_Quit();

    return 0;
}
