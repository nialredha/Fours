#include <stdio.h>
#include <stdbool.h>

#include <SDL.h>

#include "position.h"
#include "audio.h"
#include "button.h"
#include "text.h"
#include "wave.h"

#define SCREEN_WIDTH (720)
#define SCREEN_HEIGHT (295)
#define NUM_INSTRUMENTS (3)
#define MAX_CHARS (20)

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Mix16 mix = {0};

void audio_callback(void* userdata, Uint8* stream, int length)
{
    int16_t* stream16 = (int16_t*)stream;
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
        fprintf(stderr, "WARNING: input '%s', but program doesn't except input arguments.\n", argv[1]);
    }

    // SDL Audio Visual Initialization
    if (!init_av()) 
    { 
        fprintf(stderr, "ERROR in %s, line %d: failed to initialize.\n", __FILE__, __LINE__);
        exit(1); 
    }

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

    if(!audio_load_track(&kick, &mix))  
    { 
        fprintf(stderr, "ERROR in %s, line %d: failed to load kick.\n", __FILE__, __LINE__);
        exit(1); 
    } 
    if(!audio_load_track(&hat, &mix))   
    { 
        fprintf(stderr, "ERROR in %s, line %d: failed to load kick.\n", __FILE__, __LINE__);
        exit(1); 
    } 
    if(!audio_load_track(&snare, &mix)) 
    { 
        fprintf(stderr, "ERROR in %s, line %d: failed to load kick.\n", __FILE__, __LINE__);
        exit(1); 
    } 

    mix.spec.callback = audio_callback;
    if(!audio_open(&mix)) 
    { 
        fprintf(stderr, "ERROR in %s, line %d: failed to open audio device.\n", __FILE__, __LINE__);
        exit(1); 
    }; 

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
        else if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_SPACE) 
            { 
                play_selected = !play_selected; 
                audio_play(play_selected, &mix);
            }
        }

        // Clear screen to gray
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255); 
        SDL_RenderClear(renderer);

        // Render Frame

        // Play Button
        button_update_pos(85, 50, &button);
        if (add_button(&button, &play_selected, mouse.x, mouse.y, renderer, &event))
        {
            if (play_selected) { mix.playhead = 0; }
            audio_play(play_selected, &mix);
        }
        text_update(85, 45, LEFT, BOTTOM, "Play", &text);
        add_text(&text, renderer);

        // BPM Variables
        static int bpm = 60;
        static char bpm_str[MAX_CHARS];

        // BPM Decrement Button
        button_update_pos(153, 50, &button);
        if (add_button(&button, NULL, mouse.x, mouse.y, renderer, &event))
        {
            bpm--;
        }
        text_update(button.center_x, button.center_y, CENTER, CENTER, "-", &text);
        add_text(&text, renderer);

        // BPM Increment Button
        button_update_pos(187, 50, &button);
        if (add_button(&button, NULL, mouse.x, mouse.y, renderer, &event))
        {
            bpm++;
        }
        text_update(button.center_x, button.center_y, CENTER, CENTER, "+", &text);
        add_text(&text, renderer);

        // BPM Display Value
        text_update(153, 45, LEFT, BOTTOM, "BPM", &text);
        add_text(&text, renderer);

        snprintf(bpm_str, MAX_CHARS, "%d", bpm);
        text_update(192, 45, LEFT, BOTTOM, bpm_str, &text);
        add_text(&text, renderer);

        // Bars Variables
        static int bars = 0;
        static char bars_str[MAX_CHARS];

        // Bars Decrement Button
        button_update_pos(255, 50, &button);
        if (add_button(&button, NULL, mouse.x, mouse.y, renderer, &event))
        {
            if (bars > 0) { bars--; }
        }
        text_update(button.center_x, button.center_y, CENTER, CENTER, "-", &text);
        add_text(&text, renderer);

        // Bars Increment Button
        button_update_pos(289, 50, &button);
        if (add_button(&button, NULL, mouse.x, mouse.y, renderer, &event))
        {
            if (bars < 16) { bars++; }
        }
        text_update(button.center_x, button.center_y, CENTER, CENTER, "+", &text);
        add_text(&text, renderer);

        // Bars Display Value
        text_update(255, 45, LEFT, BOTTOM, "Bars", &text);
        add_text(&text, renderer);

        snprintf(bars_str, MAX_CHARS, "%d", bars);
        text_update(294, 45, LEFT, BOTTOM, bars_str, &text);
        add_text(&text, renderer);

        // Instrument Pads
        int id = 0;
        button_update_pos(85, 100, &button);
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
            text_update(button.x - 5, button.center_y, RIGHT, CENTER, instruments[i], &text);
            add_text(&text, renderer);
            button.y += button.height + 10;
        }

        // Fill mix buffer with current pad sequence
        if (!audio_fill_mix(&kick, &mix, &bpm, &pad_selected[0]))    
        { 
            fprintf(stderr, "ERROR in %s, line %d: failed to fill mix with kick.\n", __FILE__, __LINE__);
            exit(1); 
        }
        if (!audio_fill_mix(&hat, &mix, &bpm, &pad_selected[16]))    
        { 
            fprintf(stderr, "ERROR in %s, line %d: failed to fill mix with hihat.\n", __FILE__, __LINE__);
            exit(1); 
        }
        if (!audio_fill_mix(&snare, &mix, &bpm, &pad_selected[32]))  
        { 
            fprintf(stderr, "ERROR in %s, line %d: failed to fill mix with snare.\n", __FILE__, __LINE__);
            exit(1); 
        }

        // Save Button
        static int track_num = 1;
        char file_name[MAX_CHARS];
        snprintf(file_name, MAX_CHARS, "%s%d%s", "track-", track_num, ".wav");
        button_update_pos(595, 215, &button);
        if (add_button(&button, NULL, mouse.x, mouse.y, renderer, &event))
        {
            if (!audio_export_wave(file_name, bars, &mix))
            {
                fprintf(stderr, "ERROR in %s, line %d: failed to export pattern.\n", __FILE__, __LINE__);
            }
            else { track_num++; }
        }
        text_update(button.x - 5, button.center_y, RIGHT, CENTER, "Save", &text);
        add_text(&text, renderer);
            
        // Present Frame
        SDL_RenderPresent(renderer);
    }

    // Clean Up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_CloseAudioDevice(mix.device_id);
    SDL_FreeWAV((Uint8*)kick.buffer);
    SDL_FreeWAV((Uint8*)hat.buffer);
    SDL_FreeWAV((Uint8*)snare.buffer);
    free(mix.buffer);

    TTF_Quit();
    SDL_Quit();

    return 0;
}
