#include "cpu.h"
#include <iostream>

int main() {
    CPU cpu;

    cpu.initialize();

    cpu.emulateCycle();

    return 0;
}