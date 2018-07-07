/*
 * Chip8
 * Implementation of the Chip-8 CPU Core.
 */

#ifndef IMIT8_CHIP8_CHIP8_H
#define IMIT8_CHIP8_CHIP8_H

#define SCREEN_HEIGHT 32
#define SCREEN_WIDTH 64
#define MEMORY_SIZE 4096
#define NUMBER_OF_REGISTERS 16
#define STACK_DEPTH 16
#define NUMBER_OF_KEYPAD_BUTTONS 16


class chip8 {
    public:

        // Constructor. (In case you couldn't tell.)
        chip8();

        // Initialization of variables.
        bool init();
        bool load(const char* fileToLoad);

private:

        // Simulates the system memory (RAM).  This should probably
        // be moved to it's own object at some point for good OOP...
        unsigned char memory[MEMORY_SIZE];

        // Simulates the CPU registers.
        // Registers named V0 through VE.  16th register ("VF") used as "carry flag".
        unsigned char registers[NUMBER_OF_REGISTERS];

        // "Index" register.
        unsigned short index;
        // Program counter.
        unsigned short pcounter;

        // Used to store the current opcode to be processed.
        unsigned short opcode;

        // Used to simulate VRAM, this is the buffer that gets written to the display.
        unsigned char graphicsBuffer[SCREEN_HEIGHT * SCREEN_WIDTH];

        // "Interrupt timers" that the CHIP-8 uses for delays and sound purposes.
        // These timers go off every 60Hz.  They are set to some value > 0, and count down to zero.
        // When the timer reaches zero, a delay or sound is triggered.
        unsigned char delayInterruptTimer;
        unsigned char soundInterruptTimer;


        // The Chip-8 system does not have a stack, but we need one to keep track of where to return to
        // when a function call is made.
        unsigned short callStack[STACK_DEPTH];
        unsigned short callStackPointer;


        // The Chip-8 system has a keypad that uses 16 buttons, labeled in HEX (0x0-0xF).
        // This array stores the values of the keys currently being pressed.
        unsigned char keypad[NUMBER_OF_KEYPAD_BUTTONS];

};


#endif //IMIT8_CHIP8_CHIP8_H
