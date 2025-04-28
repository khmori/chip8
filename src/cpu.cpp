#include "cpu.h"
#include <string>
#include <string.h>
#include <iostream>

CPU::CPU() {
    initialize();
}

static const uint8_t font[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void CPU::initialize() {
    PC = 0x200; // set PC to 0x200
    opcode = 0; // reset opcode
    I = 0;      // reset index register
    SP = 0;     // reset stack pointer

    // clear memory
    memset(memory, 0, sizeof(memory));

    // clear display
    memset(display, 0, sizeof(display));

    // clear stack
    memset(stack, 0, sizeof(stack));
    
    // load font into memory
    for (int i = 0; i < sizeof(font); i++) {
        memory[i] = font[i];
    }
}

void CPU::loadROM(const uint8_t* program, size_t size) {

}

void CPU::emulateCycle() {
    // FETCH

    // combine two bytes into 16-bit instruction
    opcode = (memory[PC] << 8) | memory[PC+1];

    PC += 2;

    // DECODE
    uint8_t  iCd = (opcode & 0xF000) >> 12; // first nibble (4 bits)
    uint8_t  X   = (opcode & 0x0F00) >> 8;  // second nibble 
    uint8_t  Y   = (opcode & 0x00F0) >> 4;  // third nibble
    uint8_t  N   = (opcode & 0x000F);       // fourth nibble
    uint8_t  NN  = (opcode & 0x00FF);       // second byte (8 bits)
    uint16_t NNN = (opcode & 0x0FFF);       // second, third, fourth nibbles (12 bits)

    switch (iCd) {
        case 0:
            switch (NNN) {
                case 0x0E0: // clear display
                    memset(display, 0, sizeof(display));
                case 0x0EE: // return from subroutine
                    PC = stack->top();
                    stack->pop();
                    
            }
        case 1: // jump to NNN
            PC = NNN;
        case 2: // call subroutine at NNN
            // push current PC to stack, set PC to NNN
            stack->push(PC);
            PC = NNN;
        case 3:
            if (NN == V[X]) {
                PC += 2;
            }
        case 4:
            if (NN != V[X]) {
                PC += 2;
            }
        case 5:
            if (V[X] == V[Y]) {
                PC += 2;
            }
        case 6:
            V[X] = NN;
        case 7:
            V[X] += NN;
        case 9:
            if (V[X] != V[Y]) {
                PC += 2;
            }
        case 0xA:
            I = NNN;
        case 0xD:
            uint8_t x = V[X]%64;
            uint8_t y = V[Y]%32;
            uint8_t sprite_byte;
            uint8_t pixel;
            V[0xF] = 0;

            for (int row=0; row<N; row++) {
                sprite_byte = memory[I + row];

                uint8_t current_x = x;

                for (int bit=0; bit<8; bit++) {
                    // ...

                    int display_index = (y*64)+ current_x;

                    if (display_index >= (64 *32)) {
                        continue;
                    }

                    if (display[display_index] == 1) {
                        V[0xF] = 1; // collision
                    }

                    display[display_index] ^= 1; // flip pixel (XOR with 1)
                }

                current_x++;

                if (current_x >= 64) {
                    break;
                }
            }


    
    default:
        break;
    }

}