#include <cstdint>
#include <cstddef>
#include <stack>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>


#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

using namespace std;

class CPU {
public:
    CPU();
    void initialize();
    void loadROM(string romPath);
    void emulateCycle();

    bool display[64*32];
    bool keypad[16];
    bool drawFlag = false;

    const int SCALE = 10;

private: 
    uint8_t memory[4096];
    uint16_t PC; // program counter
    uint16_t opcode; // current opcode

    uint16_t I; // index register
    uint16_t stack[16]; // for subroutines/functions
    uint8_t SP; // stack pointer
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t V[16]; // variable registers 
};

