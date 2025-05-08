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
    
    // helpers
    bool isKeyPressed();
    void writeToRegister(uint8_t index, uint8_t value);
    void advancePC();

    // getters and setters
    uint8_t getDelayTimer();
    uint8_t getSoundTimer();
    void setKeypadValue(int target, int value);
    void setDisplayValue(int target, int value);
    bool getDisplayValue(int target);
    void setDrawFlag(bool value);
    bool getDrawFlag();
    void setWaitingForKey(bool value);
    bool getWaitingForKey();
    uint8_t getKeyRegister();

    const int SCALE = 10;

private: 
    uint8_t memory[4096];
    uint16_t PC; // program counter
    uint16_t opcode; // current opcode

    uint16_t I; // index register
    uint16_t stack[16]; // for subroutines/functions
    uint8_t SP; // stack pointer
    uint8_t delayTimer;
    uint8_t soundTimer;
    uint8_t V[16]; // variable registers 
    uint8_t keyRegister;

    bool display[64*32];
    bool keypad[16];
    bool drawFlag = false;
    bool waitingForKey = false;
};

