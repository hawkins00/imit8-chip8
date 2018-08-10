/**
 * Copyright (c) 2018 Chris Kim & Matt Hawkins
 * This program is licensed under the "GPLv3 License"
 * Please see the file License.md in the source
 * distribution of this software for license terms.
 */

 /*
 * Chip8
 * Implementation of the Chip-8 CPU Core.
 */

#ifndef IMIT8_CHIP8_CHIP8_H
#define IMIT8_CHIP8_CHIP8_H

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stack>
#include <vector>
#include "LogWriter.h"

#define SCREEN_HEIGHT 32
#define SCREEN_WIDTH 64
#define SCREEN_WIDTH_SIZE (SCREEN_WIDTH / 8)
#define SCREEN_SIZE 256
#define MEMORY_SIZE 4096
#define NUMBER_OF_REGISTERS 16
#define STACK_DEPTH 16
#define NUMBER_OF_KEYPAD_BUTTONS 16
#define FONT_SIZE 80
const unsigned char BYTES_PER_FONT_CHAR = 0x5;
const unsigned short CODE_START = 0x200;

class Chip8
{
    public:

        // Constructor. (In case you couldn't tell.)
        Chip8(LogWriter& logWriter);

        // Initialization of variables.
        bool init();

        // Loads the supplied file as the ROM
        bool loadFile(const std::string& fileToLoad);

        // Returns a pointer to the screen section of memory
        unsigned char* getScreen();

        // Run one cycle of the VM
        bool runCycle();

        // Does the screen need to be drawn?
        bool isDirtyScreen();

        // Update timers
        bool updateTimers();

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
        unsigned short progCounter;

        // Used to store the current opCode to be processed
        unsigned short opCode;

        // Used to simulate VRAM, this is the buffer that gets written to the display.
        unsigned char graphicsBuffer[SCREEN_HEIGHT * SCREEN_WIDTH];

        // Font to store in memory (0-F)
        unsigned char font[FONT_SIZE] = {0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70,
                                         0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0,
                                         0x90, 0x90, 0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0,
                                         0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40,
                                         0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0, 0x10, 0xF0,
                                         0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0,
                                         0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0,
                                         0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80};

        // "Interrupt timers" that the CHIP-8 uses for delays and sound purposes.
        // These timers go off every 60Hz.  They are set to some value > 0, and count down to zero.
        // When the timer reaches zero, a delay or sound is triggered.
        // TODO: ^^^ Not so sure this is correct ^^^
        unsigned char delayInterruptTimer;
        unsigned char soundInterruptTimer;

        // The Chip-8 system does not have a stack, but we need one to keep track of where to return to
        // when a function call is made.
        std::stack<unsigned short, std::vector<unsigned short>> callStack;

        // The Chip-8 system has a keypad that uses 16 buttons, labeled in HEX (0x0-0xF).
        // This array stores the values of the keys currently being pressed.
        unsigned char keypad[NUMBER_OF_KEYPAD_BUTTONS];

        // size of loaded ROM in bytes
        unsigned short romBytes;

        // display needs to be updated if dirty
        bool isDirty;

        // shared LogWriter
        LogWriter* logWriter;

        // Load font into memory
        bool loadFontSet();

        // Load ROM file into memory
        bool loadROM(std::ifstream * fin);

        // What they say on the box
        bool fetch();
        bool decodeAndExecute();

        // OpCodes are 4 hex digits. Generally we want a subset of those digits.
        unsigned char getHexDigit1(unsigned short hexShort);
        unsigned char getHexDigit2(unsigned short hexShort);
        unsigned char getHexDigit3(unsigned short hexShort);
        unsigned char getHexDigit4(unsigned short hexShort);
        unsigned char getHexDigits3and4(unsigned short hexShort);
        unsigned short getHexAddress(unsigned short hexShort); // digits 2, 3, 4

        // helper function for printing hex numbers
        std::string intToHexString(unsigned short number, int width = 4);
};

#endif //IMIT8_CHIP8_CHIP8_H
