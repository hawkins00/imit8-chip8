/*
 * Chip8
 * Implementation of the Chip-8 CPU Core.
 */

#include "chip8.h"

chip8::
chip8()
{
    std::cout << "Creating CHIP-8 CPU..." << std::endl;
    init();
}

bool chip8::
init()
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
    opCode = 0;
    progCounter = CODE_START;
    romBytes = 0;
    soundInterruptTimer = 0;


    std::cout << "Done initializing CPU..." << std::endl;
    return true;
}


// Load game file into memory at 0x200 (decimal 512)
bool chip8::
loadFile(const std::string& fileToLoad)
{
    std::cout << "Reading CHIP-8 ROM '" << fileToLoad << "'..." << std::endl;
    std::ifstream fin(fileToLoad, std::ios::in);
    bool isRomLoaded = loadROM(&fin);
    fin.close();
    if (isRomLoaded)
    {
        std::cout << "Done reading CHIP-8 ROM (" << romBytes << " bytes)..." << std::endl;
        return true;
    }
    else
    {
        std::cout << "CHIP-8 ROM was not loaded..." << std::endl;
        return false;
    }
}

// Copy font to first 80 memory locations
bool chip8::
loadFontSet()
{
    std::copy_n(font, FONT_SIZE, memory);
    return true;
}

bool chip8::
loadROM(std::ifstream* fin)
{
    char op;
    int i = CODE_START;
    for (; i < MEMORY_SIZE && !fin->eof(); ++i)
    {
        fin->read(&op, 1);
        memory[i] = static_cast<uint_fast8_t>(op);
    }
    romBytes = static_cast<uint_fast16_t>(i - CODE_START - 1);

    // If we filled the memory, but we're not at the end of the file, then the ROM is too big.
    // If the length of the ROM file is not even (or 0), it's an error (all instructions are two bytes).
    return !((i >= MEMORY_SIZE && !fin->eof()) || romBytes % 2 || !romBytes);

    /*
    // Just some testing output
    for (int j = CODE_START; j < MEMORY_SIZE; j += 2)
    {
        std::cout << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned short>(memory[j]) << std::setw(2) << static_cast<unsigned short>(memory[j+1]) << " ";
    }
    std::cout << "Done printing CHIP-8 ROM..." << std::endl;
    std::cout << std::dec << "Code size: " << romBytes << std::endl
    */
}

// Run the next cycle
bool chip8::
nextCycle()
{
    return fetch() && decode() && execute();
}

// Fetch the next opCode
bool chip8::
fetch()
{
    if (progCounter >= CODE_START + romBytes || progCounter < CODE_START || progCounter % 2) {
        return false;
    }
    opCode = static_cast<uint_fast16_t>(memory[progCounter] << 8 | memory[progCounter + 1]);
    return true;
}

// TODO: A whole bunch o' opCodes
// TODO: Maybe combine into decodeAndExecute()
// Decode the fetched opCode
bool chip8::
decode()
{
    // TODO: Change ifs to case or something even grander?
    if ((opCode >> 12) == 0x1)
    {
        progCounter = opCode & 0x0FFF;
        // check segfault (is this necessary?) or odd address
        if (progCounter < CODE_START || progCounter % 2)
        {
            return false;
        }
        std::cout << "GOTO: " << std::hex << progCounter << std::endl;
    }
    // 2XXX (subroutine call)
    else if ((opCode >> 12) == 0x2)
    {
        progCounter = opCode & 0x0FFF;

        // stack overflow or segfault or odd address
        if (callStack.size() >= STACK_DEPTH || progCounter < CODE_START || progCounter % 2)
        {
            return false;
        }

        callStack.push(progCounter);
        std::cout << "CALL: " << std::hex << progCounter << std::endl;
    }
    // FX15 (soundInterruptTimer = register[X])
    else if ((opCode & 0xF0FF) == 0xF015)
    {
        soundInterruptTimer = static_cast<uint_fast8_t>(registers[(opCode >> 8) & 0xF]);
        progCounter += 2;
        //std::cout << std::hex << ((opCode >> 8) & 0x0F) << std::endl;
        //std::cout << "Sound: " << std::hex << (int)soundInterruptTimer << std::endl;
    }
    else
    {
        progCounter += 2;
    }

    std::cout << std::hex << progCounter << std::endl;
    return true;
}

// Execute the decoded operation
bool chip8::execute()
{
    if (soundInterruptTimer > 0)
    {
        --soundInterruptTimer;
    }
    if (delayInterruptTimer > 0)
    {
        --delayInterruptTimer;
    }

    return true;
}
