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
    for (int i = 0; i < MEMORY_SIZE; ++i)
    {
        memory[i] = 0;
    }
    for (int i = 0; i < NUMBER_OF_REGISTERS; ++i)
    {
        registers[i] = 0;
        callStack[i] = 0;
    }
    for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; ++i)
    {
        graphicsBuffer[i] = 0;
    }
    index = 0;
    pcounter = 0;
    opcode = 0;
    delayInterruptTimer = 0;
    soundInterruptTimer = 0;
    callStackPointer = 0;

    // TODO:  Load font set.
    loadFontSet();
    return true;
}

// TODO:  Load game file into memory at 0x200 (decimal 512)
bool chip8::load(const std::string& fileToLoad)
{
    // Helper function.
    return false;
}

bool chip8::loadROM(const std::string& filename)
{
    // Actual work.
    return false;
}

bool chip8::loadFontSet()
{
    return false;
}
