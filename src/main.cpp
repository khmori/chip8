#include "cpu.h"
#include <iostream>

SDL_Window* window;
SDL_Renderer* renderer;

void initSDL();
void draw(const bool display[64*32]);

int main() {
    CPU cpu;
    cpu.initialize();
    cpu.loadROM("../roms/ibm_logo.ch8");

    initSDL();

    while (true) {
        cpu.emulateCycle();

        if (cpu.drawFlag) {
            draw(cpu.display);
            cpu.drawFlag = false;
        }

        SDL_Delay(16); // 60hz
    }

    SDL_Quit();
    return 0;
}

void initSDL() {
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("screen", 100, 100, 64, 32, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}


void draw(const bool display[64*32]) {
    SDL_Surface* winSurface = SDL_GetWindowSurface(window);
    SDL_LockSurface(winSurface);

    uint32_t* pixels = (uint32_t*)winSurface->pixels;
    for (int x=0; x<64; x++) {
        for (int y=0; y<32; y++) {
            bool pixel = display[y*64 + x];
            uint32_t colour;

            if (pixel) {
                colour = SDL_MapRGB(winSurface->format, 255, 255, 255);
            } else {
                colour = SDL_MapRGB(winSurface->format, 0, 0, 0);
            }

            pixels[y*64 + x] = colour;
        }
    }

    SDL_UnlockSurface(winSurface);
    SDL_UpdateWindowSurface(window);
}