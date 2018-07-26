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
    unsigned short i = CODE_START;
    for (; i < MEMORY_SIZE && !fin->eof(); ++i)
    {
        fin->read(&op, 1);
        memory[i] = static_cast<unsigned char>(op);
    }
    romBytes = i - CODE_START;
    // FIXME: romBytes might (?) be negative with 0-sized file (not good)
    romBytes--;

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
runCycle()
{
    return fetch() && decode() && execute();
}

// Fetch the next opCode
bool chip8::
fetch()
{
    if (progCounter >= CODE_START + romBytes || progCounter < CODE_START || progCounter % 2)
    {
        return false;
    }
    opCode = memory[progCounter] << 8 | memory[progCounter + 1];
    return true;
}

// TODO: A whole bunch o' opCodes
// TODO: Maybe combine into decodeAndExecute()
// Decode the fetched opCode
bool chip8::
decode()
{
    switch (getHexDigit1(opCode))
    {
        // 0x0XXX ()
        case 0x0:
            switch (getHexAddress(opCode))
            {
                // clear the screen
                case 0x0E0:
                    // graphics.clearScreen();
                    std::cout << "0x00E0: clear screen" << std::endl;
                    progCounter += 2;
                    break;
                // return from subroutine
                case 0x0EE:
                    if (callStack.empty())
                    {
                        std::cout << "0x00EE: call stack is empty" << std::endl;
                        exit(1);
                    }
                    progCounter = callStack.top();
                    callStack.pop();
                    std::cout << "0x00EE: return from subroutine" << std::endl;
                    break;
                // call to address XXX
                default:
                    std::cout << "0x0XXX: call to address XXX" << std::endl;
                    std::cout << "Not used in modern VMs" << std::endl;
                    progCounter += 2;
                    break;
            }
            break;

        // 0x1XXX (goto)
        case 0x1:
            progCounter = getHexAddress(opCode);
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
            progCounter = getHexAddress(opCode);

            // stack overflow or segfault or odd address
            if (callStack.size() > STACK_DEPTH || progCounter < CODE_START || progCounter % 2)
            {
                return false;
            }

            std::cout << "CALL: " << std::hex << progCounter << std::endl;
            break;

        // 0x3RXX (skip next opcode if registers[R] == XX
        case 0x3:
            if (registers[getHexDigit2(opCode)] == getHexDigits3and4(opCode))
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

        // 0x4RXX (skip next opCode if registers[R] != XX
        case 0x4:
            if (registers[getHexDigit2(opCode)] != getHexDigits3and4(opCode))
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

        // 0x5RS0 (skip next opCode if registers[R] == registers[S]
        case 0x5:
            switch (getHexDigit4(opCode))
            {
                case (0):
                    if (registers[getHexDigit2(opCode)] == registers[getHexDigit3(opCode)])
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

        // 0x6RXX (registers[R] = XX)
        case 0x6:
            registers[getHexDigit2(opCode)] = getHexDigits3and4(opCode);
            progCounter += 2;
            std::cout << "0x6RXX" << std::endl;
            break;

        // 0x7RXX (registers[R] += XX)
        case 0x7:
            registers[getHexDigit2(opCode)] += getHexDigits3and4(opCode);
            progCounter += 2;
            std::cout << "0x7RXX" << std::endl;
            break;

        // 0x8XXX
        case 0x8:
            std::cout << "0x8XXX" << std::endl;
            break;

        // 0x9XXX
        case 0x9:
            std::cout << "0x9XXX" << std::endl;
            break;

        // 0xAXXX
        case 0xA:
            std::cout << "0xAXXX" << std::endl;
            break;

        // 0xBXXX
        case 0xB:
            std::cout << "0xBXXX" << std::endl;
            break;

        // 0xCXXX
        case 0xC:
            std::cout << "0xCXXX" << std::endl;
            break;

        // 0xDXXX
        case 0xD:
            std::cout << "0xDXXX" << std::endl;
            break;

        // 0xEXXX
        case 0xE:
            std::cout << "0xEXXX" << std::endl;
            break;

        // 0xFRXX
        case 0xF:
            switch (getHexDigits3and4(opCode))
            {
                // 0xFR07 (registers[R] = delayInterruptTimer)
                case 0x07:
                    registers[getHexDigit2(opCode)] = delayInterruptTimer;
                    progCounter += 2;
                    std::cout << "0xFR07" << std::endl;
                    break;

                // FIXME (maybe?): Who knows if this is a correct way to get the keypress
                // 0xFR0A (execution waits for keypress, stored in registers[R])
                case 0x0A:
                    std::cout << "0xFR0A: waiting for keypress..." << std::endl;
                    unsigned char tempChar;
                    do
                    {
                        tempChar = static_cast<unsigned char>(getchar());
                    } while (!isxdigit(tempChar));
                    registers[getHexDigit2(opCode)] = tempChar;
                    progCounter += 2;
                    std::cout << "0xFR0A" << std::endl;
                    break;

                // 0xFR15 (delayInterruptTimer = registers[R])
                case 0x15:
                    delayInterruptTimer = registers[getHexDigit2(opCode)];
                    progCounter += 2;
                    //std::cout << std::hex << ((opCode >> 8) & 0x0F) << std::endl;
                    //std::cout << "Delay: " << std::hex << (int)delayInterruptTimer << std::endl;
                    break;

                // 0xFR18 (soundInterruptTimer = registers[R])
                case 0x18:
                    soundInterruptTimer = registers[getHexDigit2(opCode)];
                    progCounter += 2;
                    //std::cout << std::hex << ((opCode >> 8) & 0x0F) << std::endl;
                    //std::cout << "Sound: " << std::hex << (int)soundInterruptTimer << std::endl;
                    break;

                // 0xFR1E (index += registers[R])
                case 0x1E:
                    index += registers[getHexDigit2(opCode)];
                    progCounter += 2;
                    std::cout << "0xFR1E: Index += Reg[]" << std::endl;
                    break;

                // 0xFR29 (set index to address of sprite for character in registers[R])
                case 0x29:
                    // TODO (?): check if index is legal
                    index = getHexDigit2(opCode) * BYTES_PER_FONT_CHAR;
                    progCounter += 2;
                    std::cout << "0xFR29: index = font sprite address" << std::endl;
                    break;

                // 0xFR33 (binary-coded decimal of registers[R] stored in index, +1, +2)
                case 0x33:
                {
                    unsigned char tempNum = registers[getHexDigit2(opCode)];
                    memory[index] = tempNum / 100;
                    memory[index + 1] = tempNum / 10 % 10;
                    memory[index + 2] = tempNum % 10;
                    progCounter += 2;
                    std::cout << "0xFR33: store decimal in 3 binary addrs" << std::endl;
                }
                    break;

                // 0xFR55 (registers[0 to R] are dumped to memory starting at index)
                case 0x55:
                {
                    int lastRegister = getHexDigit2(opCode);
                    // FIXME: this causes segfaults, may just need other opCodes implemented
                    /*
                    for (int i = 0; i <= lastRegister; ++i)
                    {
                        memory[index + i] = registers[i];
                    }
                     */
                    progCounter += 2;
                    std::cout << "0xFR55: Write regs[0-R] at index" << std::endl;
                }
                    break;

                // 0xFR65 (memory starting at index copied to registers[0 to R])
                case 0x65:
                {
                    int lastRegister = getHexDigit2(opCode);
                    // FIXME: this causes segfaults, may just need other opCodes implemented
                    /*
                    for (int i = 0; i <= lastRegister; ++i)
                    {
                        registers[i] = memory[index + i];
                    }
                     */
                    progCounter += 2;
                    std::cout << "0xFR55: Write index to regs[0-R]" << std::endl;
                }
                    break;

                // opCode is not implemented, so crash already
                default:
                    std::cout << "OpCode not implemented (" << std::hex << opCode << ")" << std::endl;
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

unsigned char chip8::
getHexDigit1(unsigned short hexShort)
{
    return hexShort >> 12;
}

unsigned char chip8::
getHexDigit2(unsigned short hexShort)
{
    return (hexShort >> 8) & 0xF;
}

unsigned char chip8::
getHexDigit3(unsigned short hexShort)
{
    return (hexShort >> 4) & 0xF;
}

unsigned char chip8::
getHexDigit4(unsigned short hexShort)
{
    return hexShort & 0x000F;
}

unsigned char chip8::
getHexDigits3and4(unsigned short hexShort)
{
    return hexShort & 0x00FF;
}

unsigned short chip8::
getHexAddress(unsigned short hexShort)
{
    return hexShort & 0x0FFF;
}

