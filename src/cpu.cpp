#include "cpu.h"
#include <cstring>
#include <cstdlib>

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
    delayTimer = 0;
    soundTimer = 0;

    // clear memory
    memset(memory, 0, sizeof(memory));

    // clear display
    memset(display, 0, sizeof(display));

    // clear stack
    memset(stack, 0, sizeof(stack));

    // clear registers
    memset(V, 0, sizeof(V));

    // clear keypad
    memset(keypad, 0, sizeof(keypad));
    
    // load font into memory (starting at 0)
    for (int i = 0; i < sizeof(font); i++) {
        memory[i] = font[i];
    }
}


void CPU::emulateCycle() {
    if (waitingForKey) {
        return;
    }

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
                // 0x00E0: clear the display
                case 0x00E0: 
                    memset(display, 0, sizeof(display));
                    drawFlag = true;
                    PC += 2;
                    break;

                // 0x00EE: return from subroutine
                case 0x00EE: 
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

        // 0x1NNN: jump to NNN
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
            
        // set
        case 0x6:
            V[X] = NN;
            PC += 2;
            break;

        // add
        case 0x7:
            V[X] += NN;
            PC += 2;
            break;

        // logical/arithmetic instructions
        case 0x8: 
            switch (N) {
                case 0x0:
                    V[X] = V[Y];
                    break;

                case 0x1: // OR
                    V[X] = V[X] | V[Y];
                    break;

                case 0x2: // AND
                    V[X] = V[X] & V[Y];
                    break;

                case 0x3: // XOR
                    V[X] = V[X] ^ V[Y];
                    break;

                case 0x4: { // V[X] + V[Y]
                    int result = V[X] + V[Y];
                    V[X] = result;
                    V[0xF] = result > 255;
                    break;
                }

                case 0x5: { // V[X] - V[Y]
                    int result = V[X] - V[Y];
                    bool flag = V[X] >= V[Y];
                    V[X] = result;
                    V[0xF] = flag;
                    break;
                }

                case 0x6: {
                    bool flag = (V[X] & 1);
                    V[X] = V[Y];
                    V[X] = V[X] >> 1;
                    V[0xF] = flag;
                    break;
                }

                case 0x7: {
                    int result = V[Y] - V[X];
                    bool flag = V[Y] >= V[X];
                    V[X] = result;
                    V[0xF] = flag;
                    break;
                }

                case 0xE: {
                    bool flag = (V[X] & 0x80) >> 7;
                    V[X] = V[Y];
                    V[X] = V[X] << 1;
                    V[0xF] = flag;
                    break;
                }
            }
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

        case 0xB:
            // COSMAC VIP interpreter
            PC = NNN + V[0];
            break;

        case 0xC: { 
            int r = rand() % 256;
            V[X] = r & NN;
            PC += 2;
            break;
        }

            
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

        // EXNN: keypad instructions
        case 0xE: 
            switch (NN) {
                // EX9E: skip one instruction if key corresponding to V[X] is pressed
                case 0x9E: {
                    if (keypad[V[X]]) {
                        PC += 2;
                    }
                    PC += 2;
                    break;
                }
                
                // EXA1: skip one instruction if key corresponding to V[X] is not pressed
                case 0xA1: {
                    if (!keypad[V[X]]) {
                        PC += 2;
                    }
                    PC += 2;
                    break;
                }
            }
            break;
        
        // timer instructions
        case 0xF:
            switch (NN) {
                case 0x07:
                    V[X] = delayTimer;
                    PC += 2;
                    break;

                case 0x15:
                    delayTimer = V[X];
                    PC += 2;
                    break;
                
                case 0x18:
                    soundTimer = V[X];
                    PC += 2;
                    break;

                case 0x1E:
                    I += V[X];
                    PC += 2;
                    break;

                // FX0A: get key
                // stops executing instructions and waits for key input
                case 0x0A:
                    // PC -= 2;
                    waitingForKey = true;
                    keyRegister = X;
                    break;
                
                case 0x29:
                    I = V[X] * 5;
                    PC += 2;
                    break;
                
                case 0x33:
                    memory[I+2] = V[X] % 10;    
                    memory[I+1] = (V[X]/10) % 10;
                    memory[I] = V[X]/100;
                    PC += 2;
                    break;

                case 0x55:
                    for (int i=0; i<=X; i++) {
                        memory[I+i] = V[i];
                    }
                    PC += 2;
                    break;
                
                case 0x65:
                    for (int i=0; i<=X; i++) {
                        V[i] = memory[I + i];
                    }
                    PC += 2;
                    break;
            }
            break;

        default:
            cout << "unknown opcode: 0x" << hex << opcode << endl;
            break;
    }

}

void CPU::updateTimers() {
    if (delayTimer > 0) {
        delayTimer--;
    }

    if (soundTimer > 0) {
        soundTimer--;
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

    // TESTING KEYPAD
    // memory[0x1FF] = 2;
}

bool CPU::isKeyPressed() {
    for (int i=0; i<sizeof(keypad); i++) {
        if (keypad[i]) {
            return true;
        }
    }
    return false;
}

void CPU::writeToRegister(uint8_t index, uint8_t value) {
    V[index] = value;
}

void CPU::advancePC() {
    PC += 2;
}

uint8_t CPU::getDelayTimer() {
    return delayTimer;
}

uint8_t CPU::getSoundTimer() {
    return soundTimer;
}

void CPU::setKeypadValue(int target, int value) {  
    keypad[target] = value; 
}

void CPU::setDisplayValue(int target, int value) {
    display[target] = value;
}

bool CPU::getDisplayValue(int target) {
    return display[target];
}

void CPU::setDrawFlag(bool value) {
    drawFlag = value;
}

bool CPU::getDrawFlag() {
    return drawFlag;
}

void CPU::setWaitingForKey(bool value) {
    waitingForKey = value;
}

bool CPU::getWaitingForKey() {
    return waitingForKey;
}

uint8_t CPU::getKeyRegister() {
    return keyRegister;
}