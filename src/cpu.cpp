#include "cpu.h"
#include <iomanip>

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

    // clear registers
    memset(V, 0, sizeof(V));
    
    // load font into memory (starting at 0)
    for (int i = 0; i < sizeof(font); i++) {
        memory[i] = font[i];
    }
}


void CPU::emulateCycle() {
    // FETCH
    // combine two bytes into 16-bit instruction
    opcode = (memory[PC] << 8) | (memory[PC+1]);

    if (PC >= 4096) {
        cout << "PC is out of bounds" << endl;
        return;
    }

    // DECODE
    uint8_t  iCd = (opcode & 0xF000) >> 12; // first nibble (4 bits)
    uint8_t  X   = (opcode & 0x0F00) >> 8;  // second nibble 
    uint8_t  Y   = (opcode & 0x00F0) >> 4;  // third nibble
    uint8_t  N   = (opcode & 0x000F);       // fourth nibble
    uint8_t  NN  = (opcode & 0x00FF);       // second byte (8 bits)
    uint16_t NNN = (opcode & 0x0FFF);       // second, third, fourth nibbles (12 bits)

    // check PC, opcode, SP, registers
    printf("%.4X %.4X %.2X ", PC, opcode, SP);
    for (int i = 0; i < 15; i++)
    {
        printf("%.2X ", V[i]);
    }
    printf("\n");

    switch (iCd) {
        case 0x0:
            switch (opcode) {
                case 0x00E0: // clear display
                    memset(display, 0, sizeof(display));
                    drawFlag = true;
                    PC += 2;
                    break;

                case 0x00EE: // return from subroutine
                    if (SP > 0) {
                        SP--;
                        PC = stack[SP];
                        PC += 2;
                    } else {
                        std::cerr << "stack underflow, 0x00EE";
                        PC += 2;
                    }
                    break;

                default:
                    std::cerr << "Unknown 0x0 opcode: " << std::hex << opcode << std::endl;
                    PC += 2;  // don't get stuck
                    break;
            }
            break;

        case 0x1: // jump to NNN
            PC = NNN;
            break;

        case 0x2: // call subroutine at NNN
            // push current PC to stack, set PC to NNN
            if (SP < 16) {
                stack[SP] = PC;
                SP++;
                PC = NNN;
            } else {
                std::cerr << "stack underflow, 0x2NN";
                PC += 2;
            }
            break;

        case 0x3:
            PC += 2;
            if (V[X] == NN) {
                PC += 2;
            }
            break;

        case 0x4:
            PC += 2;
            if (V[X] != NN) {
                PC += 2;
            }
            break;

        case 0x5:
            PC += 2;
            if (V[X] == V[Y]) {
                PC += 2;
            }
            break;
            
        case 0x6:
            V[X] = NN;
            PC += 2;
            break;

        case 0x7:
            V[X] += NN;
            PC += 2;
            break;

        case 0x9:
            PC += 2;
            if (V[X] != V[Y]) {
                PC += 2;
            }
            break;

        case 0xA:
            I = NNN;
            PC += 2;
            break;

        case 0xD: {
            uint8_t x = V[X]%64;
            uint8_t y = V[Y]%32;
            V[0xF] = 0;

            for (int row=0; row<N; row++) {
                uint8_t spriteByte = memory[I + row];
                uint8_t currentX = x;

                for (int bit=0; bit<8; bit++) {
                    if (x >= 64) {
                        break;
                    }

                    uint8_t spritePixel = (spriteByte >> (7 - bit)) & 1; // nth bit from left
                    int displayIndex = y * 64 + currentX;

                    if (spritePixel) {
                        if (display[displayIndex]) {
                            display[displayIndex] = 0;
                            V[0xF] = 1;
                        } else {
                            display[displayIndex] = 1;
                        }
                    }

                    currentX++;
                }

                y++;

                if (y >= 32) {
                    break;
                }
            }

            drawFlag = true;
            PC += 2;
            break;
        }

        default:
            cout << "unknown opcode: 0x" << hex << opcode << endl;
            break;
    }

}

void CPU::loadROM(string romPath) {
    ifstream rom(romPath, ios::binary | ios::in);
    
    // start reading bytes at address 0x200
    uint16_t index = 0x200;
    char byte; // a char is one byte
    while (rom.get(byte) && index < 4096) { // can only call get with chars
        memory[index] = (uint8_t) byte; // cast char to uint8_t
        index++;
    }

    rom.close();
    cout << "loaded " << index - 0x200 << " bytes into memory\n";
}