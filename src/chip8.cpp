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

    delayInterruptTimer = 0;
    index = 0;
    opcode = 0;
    progCounter = CODE_START;
    romBytes = 0;
    soundInterruptTimer = 0;


    std::cout << "Done initializing CPU..." << std::endl;
    return true;
}


// Load game file into memory at 0x200 (decimal 512)
bool chip8::loadFile(const std::string& fileToLoad)
{
    std::cout << "Reading CHIP-8 ROM " << fileToLoad << "..." << std::endl;
    std::ifstream fin(fileToLoad, std::ios::in);
    loadROM(&fin);
    fin.close();
    std::cout << "Done reading CHIP-8 ROM..." << std::endl;
    return true;
}

// Copy font to first 80 memory locations
bool chip8::loadFontSet()
{
    std::copy_n(font, FONT_SIZE, memory);
    return true;
}

bool chip8::loadROM(std::ifstream * fin)
{
    char op;
    int i = CODE_START;
    for (; i < MEMORY_SIZE && !fin->eof(); ++i)
    {
        fin->read(&op, 1);
        memory[i] = (uint_fast8_t)op;
    }
    if (i >= MEMORY_SIZE && !fin->eof())
    {
        return false;
    }

    romBytes = (uint_fast16_t)(i - CODE_START - 1);

    /*
    for (int j = CODE_START; j < MEMORY_SIZE; j += 2)
    {
        std::cout << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned short>(memory[j]) << std::setw(2) << static_cast<unsigned short>(memory[j+1]) << " ";
    }
    std::cout << "Done printing CHIP-8 ROM..." << std::endl;
    std::cout << std::dec << "Code size: " << romBytes << std::endl
    */

    return true;
}
