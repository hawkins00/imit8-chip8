/*
 * Chip8
 * Implementation of the Chip-8 CPU Core.
 */

#include "chip8.h"

chip8::chip8()
{
    std::cout << "Creating CHIP-8 CPU..." << std::endl;
    init();
}

bool chip8::init()
{
    std::cout << "Initializing CPU..." << std::endl;

    for (int i = 0; i < NUMBER_OF_REGISTERS; ++i)
    {
        registers[i] = 0;
    }

    for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; ++i)
    {
        graphicsBuffer[i] = 0;
    }

    while (!callStack.empty())
    {
        callStack.pop();
    }

    loadFontSet(); // load font to memory, then zero the rest
    for (int i = FONT_SIZE; i < MEMORY_SIZE; ++i)
    {
        memory[i] = 0;
    }

    index = 0;
    progCounter = CODE_START;
    opcode = 0;
    delayInterruptTimer = 0;
    soundInterruptTimer = 0;

    return true;
}


// TODO:  Load game file into memory at 0x200 (decimal 512)
bool chip8::loadFile(const std::string& fileToLoad)
{
    // Helper function.
    return false;
}

// Copy font to first 80 memory locations
bool chip8::loadFontSet()
{
    std::copy_n(font, FONT_SIZE, memory);
    return true;
}

bool chip8::loadROM(const std::string& filename)
{
    // Actual work.
    return false;
}
