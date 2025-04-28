#include <cstdint>
#include <cstddef>
#include <stack>

class CPU {
public:
    CPU();
    void initialize();
    void loadROM(const uint8_t* program, size_t size);
    void emulateCycle();

    bool display[64*32];
    bool keypad[16];

private: 
    uint8_t memory[4096];
    uint16_t PC; // program counter
    uint8_t opcode; // current opcode

    uint16_t I; // index register
    std::stack<uint16_t> stack[16]; // for subroutines/functions
    uint8_t SP; // stack pointer
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t V[16]; // variable registers    
};

