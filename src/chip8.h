/*
 * Chip8
 * Implementation of the Chip-8 CPU Core.
 */

#ifndef IMIT8_CHIP8_CHIP8_H
#define IMIT8_CHIP8_CHIP8_H

#include <iostream>

#define SCREEN_HEIGHT 32
#define SCREEN_WIDTH 64
#define MEMORY_SIZE 4096
#define NUMBER_OF_REGISTERS 16
#define STACK_DEPTH 16
#define NUMBER_OF_KEYPAD_BUTTONS 16

class chip8
{
    public:

        // Constructor. (In case you couldn't tell.)
        chip8();

        // Initialization of variables.
        bool init();

        bool load(const std::string& fileToLoad);

    private:

        // Simulates the system memory (RAM).  This should probably
        // be moved to it's own object at some point for good OOP...
        uint_fast8_t memory[MEMORY_SIZE];

        // Simulates the CPU registers.
        // Registers named V0 through VE.  16th register ("VF") used as "carry flag".
        uint_fast8_t registers[NUMBER_OF_REGISTERS];

        // "Index" register.
        uint_fast16_t index;

        // Program counter.
        uint_fast16_t pcounter;

        // Used to store the current opcode to be processed.
        uint_fast16_t opcode;

        // Used to simulate VRAM, this is the buffer that gets written to the display.
        uint_fast8_t graphicsBuffer[SCREEN_HEIGHT * SCREEN_WIDTH];

        // "Interrupt timers" that the CHIP-8 uses for delays and sound purposes.
        // These timers go off every 60Hz.  They are set to some value > 0, and count down to zero.
        // When the timer reaches zero, a delay or sound is triggered.
        uint_fast8_t delayInterruptTimer;
        uint_fast8_t soundInterruptTimer;

        // The Chip-8 system does not have a stack, but we need one to keep track of where to return to
        // when a function call is made.
        uint_fast16_t callStack[STACK_DEPTH];
        uint_fast16_t callStackPointer;

        // The Chip-8 system has a keypad that uses 16 buttons, labeled in HEX (0x0-0xF).
        // This array stores the values of the keys currently being pressed.
        uint_fast8_t keypad[NUMBER_OF_KEYPAD_BUTTONS];

        bool loadFontSet();
        bool loadROM(const std::string& filename);

};

#endif //IMIT8_CHIP8_CHIP8_H
