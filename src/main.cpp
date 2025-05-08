#include "cpu.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <iostream>

const int SCALE = 10;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Uint32 lastTimerUpdate; // number of milliseconds since initialization

void initSDL();
void draw(CPU& cpu);
void handleKeyEvent(SDL_Event event, CPU& cpu);

int main() {
    CPU cpu;
    cpu.initialize();
    cpu.loadROM("../roms/4-flags.ch8");

    initSDL();

    while (true) {
        cpu.emulateCycle();
        SDL_Event event;

        // draw to screen
        if (cpu.getDrawFlag()) {
            draw(cpu);
            cpu.setDrawFlag(false);
        }
        
        // handle key inputs
        while(SDL_PollEvent(&event)) {
            handleKeyEvent(event, cpu);
        }

        // handle timers
        // if 1/60 of a second (1000/60 milliseconds) has passed, update timers
        // (timers are decremented 60 times per second)
        Uint32 now = SDL_GetTicks();
        if (now - lastTimerUpdate >= 1000/60) {
            cpu.updateTimers();
            lastTimerUpdate = now;
        }

        SDL_Delay(1); 
    }

    SDL_Quit();
    return 0;
}

void initSDL() {
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("screen", 100, 100, 640, 320, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    lastTimerUpdate = SDL_GetTicks();
}

void draw(CPU& cpu) {   
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
    SDL_RenderClear( renderer );
    
    SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );

    for (int x=0; x<64; x++) {
        for (int y=0; y<32; y++) {
            bool pixel = cpu.getDisplayValue(y*64 + x);
            
            if (pixel) {
                SDL_Rect rect;
                rect.h = SCALE;
                rect.w = SCALE;
                rect.x = x * SCALE;
                rect.y = y * SCALE;
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    // render the rect to the screen
    SDL_RenderPresent(renderer);

    return;
}

void handleKeyEvent(SDL_Event event, CPU& cpu) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        cout << event.key.keysym.sym << endl; 
        bool isDown = (event.type == SDL_KEYDOWN);
        int target;

        switch (event.key.keysym.sym) {
            case SDLK_1: target = 0; break;
            case SDLK_2: target = 1;break;
            case SDLK_3: target = 2; break;
            case SDLK_4: target = 3; break;
            case SDLK_q: target = 4; break;
            case SDLK_w: target = 5; break;
            case SDLK_e: target = 6; break;
            case SDLK_r: target = 7; break;
            case SDLK_a: target = 8; break;
            case SDLK_s: target = 9; break;
            case SDLK_d: target = 10; break;
            case SDLK_f: target = 11; break;
            case SDLK_z: target = 12; break;
            case SDLK_x: target = 13; break;
            case SDLK_c: target = 14; break;
            case SDLK_v: target = 15; break;
            default:
                cout << "unknown input: ";
                cout << event.key.keysym.sym << endl;
                break;
        }

        cpu.setKeypadValue(target, isDown);

        if (cpu.getWaitingForKey()) { // FXOA instruction
            cpu.writeToRegister(cpu.getKeyRegister(), target);
            cpu.setWaitingForKey(false);
            cpu.advancePC();
        }

    }
}