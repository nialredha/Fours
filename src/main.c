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

char* instruments[NUM_INSTRUMENTS] = {"Kick", "HiHat", "Snare"};

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
        stream16[i] = (int16_t)mix.buffer[mix.playhead];
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
    Button button = button_new_default(0, 0, LEFT, TOP, NULL);
    Slider slider = slider_new_default(0, 0, LEFT, BOTTOM);
    bool play_selected = false;
    bool pad_selected[NUM_PADS] = {false};
    bool slider_selected[NUM_INSTRUMENTS] = {false};

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
        else if (event.type == SDL_MOUSEBUTTONUP)
        {
            for (int i = 0; i < NUM_INSTRUMENTS; i++)
            {
                if (slider_selected[i])
                {
                    slider_selected[i] = false;
                }
            }
        }

        // Clear screen to gray
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255); 
        SDL_RenderClear(renderer);

        // Render Frame

        // Play Button
        button.rect.x = 93; button.rect.y = 50; button.selected = &play_selected;
        if (add_button(&button, &mouse, renderer, &event))
        {
            if (play_selected) { mix.playhead = 0; }
            audio_play(play_selected, &mix);
        }
        text.x = 93; text.y = 45; text.ref.x = LEFT; text.ref.y = BOTTOM; text.value = "Play";
        add_text(&text, renderer);

        // BPM Variables
        static int bpm = 60;
        static char bpm_str[MAX_CHARS];

        // BPM Decrement Button
        button.rect.x += 68; button.selected = NULL;
        if (add_button(&button, &mouse, renderer, &event))
        {
            bpm--;
        }
        text.x = button.rect.cx; text.y = button.rect.cy; 
        text.ref.x = CENTER; text.ref.y = CENTER; text.value = "-";
        add_text(&text, renderer);

        // BPM Display Name
        text.x = button.rect.x; text.y = button.rect.y-5; 
        text.ref.x = LEFT; text.ref.y = BOTTOM; text.value = "BPM";
        add_text(&text, renderer);

        // BPM Increment Button
        button.rect.x += 34; 
        if (add_button(&button, &mouse, renderer, &event))
        {
            bpm++;
        }
        text.x = button.rect.cx; text.y = button.rect.cy; 
        text.ref.x = CENTER; text.ref.y = CENTER; text.value = "+";
        add_text(&text, renderer);

        // BPM Display Value
        snprintf(bpm_str, MAX_CHARS, "%d", bpm);
        text.x = button.rect.x+5; text.y = button.rect.y-5; 
        text.ref.x = LEFT; text.ref.y = BOTTOM; text.value = bpm_str;
        add_text(&text, renderer);

        // Bars Variables
        static int bars = 0;
        static char bars_str[MAX_CHARS];

        // Bars Decrement Button
        button.rect.x += 68; 
        if (add_button(&button, &mouse, renderer, &event))
        {
            if (bars > 0) { bars--; }
        }
        text.x = button.rect.cx; text.y = button.rect.cy; 
        text.ref.x = CENTER; text.ref.y = CENTER; text.value = "-";
        add_text(&text, renderer);

        // Bars Display Name
        text.x = button.rect.x; text.y = button.rect.y-5; 
        text.ref.x = LEFT; text.ref.y = BOTTOM; text.value = "Bars";
        add_text(&text, renderer);

        // Bars Increment Button
        button.rect.x += 34; 
        if (add_button(&button, &mouse, renderer, &event))
        {
            if (bars < 16) { bars++; }
        }
        text.x = button.rect.cx; text.y = button.rect.cy; 
        text.ref.x = CENTER; text.ref.y = CENTER; text.value = "+";
        add_text(&text, renderer);

        // Bars Display Value
        snprintf(bars_str, MAX_CHARS, "%d", bars);
        text.x = button.rect.x+5; text.y = button.rect.y-5; 
        text.ref.x = LEFT; text.ref.y = BOTTOM; text.value = bars_str;
        add_text(&text, renderer);

        // Instrument Pads
        static float track_volume[NUM_INSTRUMENTS] = {MAX_VOLUME, MAX_VOLUME, MAX_VOLUME};

        int id = 0;
        text.ref.x = RIGHT; text.ref.y = CENTER;
        button.rect.x = 93; button.rect.y = 100;
        slider.rect.x = 637; slider.rect.y = 192;
        for (int i = 0; i < NUM_INSTRUMENTS; i++)
        {
            for (int j = 0; j < NUM_STEPS; j++)
            {
                id = (NUM_STEPS*i) + j;
                button.selected = &pad_selected[id];
                add_button(&button, &mouse, renderer, &event);
                button.rect.x += button.rect.w + 10;
            }


            slider.rect.x += slider.rect.w + 2;
            slider.button.rect.x = slider.rect.x; 
            slider.fill_height = (int)(track_volume[i] / MAX_VOLUME * slider.rect.h);
            slider.button.rect.y = slider.rect.y - slider.fill_height; 
            if (add_slider(&slider, &mouse, renderer, &event))
            {
                slider_selected[i] = true;
            }
            if (slider_selected[i])
            {
                if (mouse.y < slider.rect.y && mouse.y > slider.rect.y - slider.rect.h)
                {
                    float new_height = (float)(slider.rect.y - mouse.y);
                    float percent_full = new_height / (float)slider.rect.h;
                    track_volume[i] = percent_full * MAX_VOLUME;
                }
                else if (mouse.y < slider.rect.y - slider.rect.h)
                {
                    track_volume[i] = MAX_VOLUME;
                }
                else if (mouse.y > slider.rect.y)
                {
                    track_volume[i] = 0.0;
                }
            }

            button.rect.x = 93;
            text.x = button.rect.x - 5; text.y = button.rect.cy; text.value = instruments[i];
            add_text(&text, renderer);
            button.rect.y += button.rect.h + 10;
        }

        // Fill mix buffer with current pad sequence
        audio_fill_mix(&kick, &mix, &bpm, &pad_selected[0], track_volume[0]);
        audio_fill_mix(&hat, &mix, &bpm, &pad_selected[16], track_volume[1]);
        audio_fill_mix(&snare, &mix, &bpm, &pad_selected[32], track_volume[2]);

        // Save Button
        static int track_num = 1;
        char file_name[MAX_CHARS];
        snprintf(file_name, MAX_CHARS, "%s%d%s", "track-", track_num, ".wav");

        button.rect.x = 603; button.rect.y = 215; button.selected = NULL;
        if (add_button(&button, &mouse, renderer, &event))
        {
            if (!audio_export_wave(file_name, bars, &mix))
            {
                fprintf(stderr, "ERROR in %s, line %d: failed to export pattern.\n", __FILE__, __LINE__);
            }
            else { track_num++; }
        }
        text.x = button.rect.x - 5; text.y = button.rect.cy; text.value = "Save";
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
