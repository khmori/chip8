#include "cpu.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <iostream>

const int SCALE = 10;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Uint32 lastTimerUpdate; // number of milliseconds since initialization
uint8_t keymap[16] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
};

void initSDL();
void draw(CPU& cpu);
void handleKeyEvent(SDL_Event event, CPU& cpu);

int main() {
    CPU cpu;
    cpu.initialize();
    cpu.loadROM("../roms/pong.ch8");

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

        for (int i=0; i<16; i++) {
            if (event.key.keysym.sym == keymap[i]) {
                cpu.setKeypadValue(i, isDown);
            }
        }

        if (cpu.getWaitingForKey() && event.type == SDL_KEYUP) { // FXOA instruction
            cpu.writeToRegister(cpu.getKeyRegister(), target);
            cpu.setWaitingForKey(false);
            cpu.advancePC();
            cout << "key pressed; PC advanced" << endl;
        }

    }
}