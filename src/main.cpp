#include "cpu.h"
#include <SDL2/SDL_events.h>
#include <atomic>
#include <iostream>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int scale = 10;

void initSDL();
void draw(CPU& cpu);
void handleKeyEvent(SDL_Event event, CPU& cpu);

int main() {
    CPU cpu;
    cpu.initialize();
    cpu.loadROM("../roms/ibm_logo.ch8");

    initSDL();

    while (true) {
        cpu.emulateCycle();
        SDL_Event event;

        if (cpu.getDrawFlag()) {
            draw(cpu);
            cpu.setDrawFlag(false);
        }
        
        while(SDL_PollEvent(&event)) {
            handleKeyEvent(event, cpu);
        }

        SDL_Delay(1000/cpu.getDelayTimer()); 
    }

    SDL_Quit();
    return 0;
}

void initSDL() {
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("screen", 100, 100, 640, 320, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

void draw(CPU& cpu) {   
    // window = SDL_CreateWindow("screen", 100, 100, 640, 320, SDL_WINDOW_SHOWN);
    // renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
    SDL_RenderClear( renderer );
    
    SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );

    for (int x=0; x<64; x++) {
        for (int y=0; y<32; y++) {
            bool pixel = cpu.getDisplayValue(y*64 + x);
            
            if (pixel) {
                SDL_Rect rect;
                rect.h = scale;
                rect.w = scale;
                rect.x = x * scale;
                rect.y = y * scale;
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    // render the rect to the screen
    SDL_RenderPresent(renderer);

    // wait for 0.5 sec
    SDL_Delay( 500 );

    return;
}

void handleKeyEvent(SDL_Event event, CPU& cpu) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        cout << event.key.keysym.sym << endl; 
        bool isDown = (event.type == SDL_KEYDOWN);

        switch (event.key.keysym.sym) {
            case SDLK_1: cpu.setKeypadValue(0, isDown); break;
            case SDLK_2: cpu.setKeypadValue(1, isDown); break;
            case SDLK_3: cpu.setKeypadValue(2, isDown); break;
            case SDLK_4: cpu.setKeypadValue(3, isDown); break;
            case SDLK_q: cpu.setKeypadValue(4, isDown); break;
            case SDLK_w: cpu.setKeypadValue(5, isDown); break;
            case SDLK_e: cpu.setKeypadValue(6, isDown); break;
            case SDLK_r: cpu.setKeypadValue(7, isDown); break;
            case SDLK_a: cpu.setKeypadValue(8, isDown); break;
            case SDLK_s: cpu.setKeypadValue(9, isDown); break;
            case SDLK_d: cpu.setKeypadValue(10, isDown); break;
            case SDLK_f: cpu.setKeypadValue(11, isDown); break;
            case SDLK_z: cpu.setKeypadValue(12, isDown); break;
            case SDLK_x: cpu.setKeypadValue(13, isDown); break;
            case SDLK_c: cpu.setKeypadValue(14, isDown); break;
            case SDLK_v: cpu.setKeypadValue(15, isDown); break;
        }

        if (cpu.getWaitingForKey()) {
            cpu.setWaitingForKey(true);
        }
    }
}