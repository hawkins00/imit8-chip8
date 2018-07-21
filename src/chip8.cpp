/*
 * Chip8
 * Implementation of the Chip-8 CPU Core.
 */

// XXX: seriously rethink this uint_fastX_t junk :/

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
    switch (opCode >> 12)
    {
        case 0x0:
            // 0XXX ()
            std::cout << "0x0" << std::endl;
            break;

        // 0x1XXX (goto)
        case 0x1:
            progCounter = opCode & 0x0FFF;
            // check segfault (is this necessary?) or odd address
            if (progCounter < CODE_START || progCounter % 2)
            {
                return false;
            }
            std::cout << "GOTO: " << std::hex << progCounter << std::endl;
        break;

        // 0x2XXX (subroutine call)
        case 0x2:
            callStack.push(progCounter);
            progCounter = opCode & 0x0FFF;

            // stack overflow or segfault or odd address
            if (callStack.size() > STACK_DEPTH || progCounter < CODE_START || progCounter % 2)
            {
                return false;
            }

            std::cout << "CALL: " << std::hex << progCounter << std::endl;
            break;

        // 0x3RXX (skip next opcode if registers[R] == XX
        case 0x3:
            if (registers[(opCode >> 8) & 0xF] == opCode & 0xFF)
            {
                progCounter += 4;
                std::cout << "0x3 (skip next)" << std::endl;
            }
            else
            {
                progCounter += 2;
                std::cout << "0x3 (don't skip next)" << std::endl;
            }
            break;

        // 0x4RXX (skip next opcode if registers[R] != XX
        case 0x4:
            if (registers[(opCode >> 8) & 0xF] != opCode & 0xFF)
            {
                progCounter += 4;
                std::cout << "0x4RXX (skip next)" << std::endl;
            }
            else
            {
                progCounter += 2;
                std::cout << "0x4RXX (don't skip next)" << std::endl;
            }
            break;

        // 0x5RS0 (skip next opcode if registers[R] == registers[S]
        case 0x5:
            switch (opCode & 0xF)
            {
                case (0):
                    if (registers[(opCode >> 8) & 0xF] == registers[(opCode >> 4)] & 0xF)
                    {
                        progCounter += 4;
                        std::cout << "0x5RS0 (skip next)" << std::endl;
                    }
                    else
                    {
                        progCounter += 2;
                        std::cout << "0x5RS0 (don't skip next)" << std::endl;
                    }
                    break;
                default:
                    // TODO: error
                    return false;
            }
            break;

        // 0x6RXX (set registers[R] = XX)
        case 0x6:
            registers[(opCode >> 8) & 0xF] = static_cast<uint_fast8_t>(opCode & 0xFF);
            progCounter += 2;
            std::cout << "0x6RXX" << std::endl;
            break;

        // 0xFRXX
        case 0xF:
            switch (opCode & 0xFF)
            {
                // 0xFR07 (registers[R] = delayInterruptTimer)
                case 0x07:
                    registers[(opCode >> 8) & 0xF] = delayInterruptTimer;
                    progCounter += 2;
                    std::cout << "0xFR07" << std::endl;
                    break;

                // FIXME (maybe?): Who knows if this is a correct way to get the keypress
                // 0xFR0A (execution waits for keypress, stored in registers[R])
                case 0x0A:
                    std::cout << "0xFR0A: waiting for keypress..." << std::endl;
                    uint_fast8_t tempChar;
                    do
                    {
                        tempChar = static_cast<uint_fast8_t>(getchar() & 0xFF);
                    } while (!isxdigit(tempChar));
                    registers[(opCode >> 8) & 0xF] = tempChar;
                    progCounter += 2;
                    std::cout << "0xFR0A" << std::endl;
                    break;

                // 0xFR15 (delayInterruptTimer = registers[R])
                case 0x15:
                    delayInterruptTimer = static_cast<uint_fast8_t>(registers[(opCode >> 8) & 0xF]);
                    progCounter += 2;
                    //std::cout << std::hex << ((opCode >> 8) & 0x0F) << std::endl;
                    //std::cout << "Delay: " << std::hex << (int)delayInterruptTimer << std::endl;
                    break;

                // 0xFR18 (soundInterruptTimer = registers[R])
                case 0x18:
                    soundInterruptTimer = static_cast<uint_fast8_t>(registers[(opCode >> 8) & 0xF]);
                    progCounter += 2;
                    //std::cout << std::hex << ((opCode >> 8) & 0x0F) << std::endl;
                    //std::cout << "Sound: " << std::hex << (int)soundInterruptTimer << std::endl;
                    break;

                // 0xFR1E (index += registers[R])
                case 0x1E:
                    index += registers[(opCode >> 8) & 0xF];
                    progCounter += 2;
                    std::cout << "0xFR1E: Index += Reg[]" << std::endl;
                    break;

                // 0xFR29 (set index to address of sprite for character in registers[R])
                case 0x29:
                    // TODO (?): check if index is legal
                    index = static_cast<uint_fast16_t>(registers[(opCode >> 8) & 0xF] * 5);
                    progCounter += 2;
                    std::cout << "0xFR29: index = font sprite address" << std::endl;
                    break;

                // 0xFR33 (set index to address of sprite for character in registers[R])
                case 0x33:
                { // scope limiter ({), so tempNum doesn't cross initialization
                    uint_fast8_t tempNum = registers[(opCode >> 8) & 0xF];
                    memory[index] = static_cast<uint_fast8_t>(tempNum / 100);
                    memory[index + 1] = static_cast<uint_fast8_t>(tempNum / 10 % 10);
                    memory[index + 2] = static_cast<uint_fast8_t>(tempNum % 10);
                    progCounter += 2;
                    std::cout << "0xFR33" << std::endl;
                    break;
                }

                // opCode is not implemented, so crash already
                default:
                    std::cout << "OpCode not implemented" << std::endl;
                    return false;
            }
            break;

        default:
        {
            progCounter += 2;
            std::cout << "default" << std::endl;
        }
    }

    std::cout << "PC: " << std::hex << progCounter << std::endl;
    return true;
}

// TODO: Combine with decode() ?
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
