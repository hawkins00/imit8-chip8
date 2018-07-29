/*
 * Chip8
 * Implementation of the Chip-8 CPU Core.
 */

#include "chip8.h"

chip8::
chip8()
{
    //std::cout << "Creating CHIP-8 CPU..." << std::endl;
    init();
}

bool chip8::
init()
{
    //std::cout << "Initializing CPU..." << std::endl;

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


    //std::cout << "Done initializing CPU..." << std::endl;
    return true;
}


// Load game file into memory at 0x200 (decimal 512)
bool chip8::
loadFile(const std::string& fileToLoad)
{
    //std::cout << "Reading CHIP-8 ROM '" << fileToLoad << "'..." << std::endl;
    std::ifstream fin(fileToLoad, std::ios::in);
    bool isRomLoaded = loadROM(&fin);
    fin.close();
    if (isRomLoaded)
    {
        //std::cout << "Done reading CHIP-8 ROM (" << romBytes << " bytes)..." << std::endl;
        return true;
    }
    else
    {
        //std::cout << "CHIP-8 ROM was not loaded..." << std::endl;
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
    // If the length of the ROM file is 0, it's an error (all instructions are two bytes).
    return !((i >= MEMORY_SIZE && !fin->eof()) || !romBytes); // || romBytes % 2);

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

// Get the starting location of vram
unsigned char * chip8::
getScreen()
{
    return &memory[SCREEN_START];
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
//    if (progCounter >= CODE_START + romBytes || progCounter < CODE_START || progCounter % 2)
    if (progCounter % 2)
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
                // TODO
                // clear the screen
                case 0x0E0:
                    for (int i = SCREEN_START; i < MEMORY_SIZE; ++i)
                    {
                        memory[i] = 0;
                    }
                    progCounter += 2;
                    std::cout << "0x00E0: clear screen" << std::endl;
                    break;
                // return from subroutine
                case 0x0EE:
                    if (callStack.empty())
                    {
                        //std::cout << "0x00EE: call stack is empty" << std::endl;
                        exit(1);
                    }
                    progCounter = callStack.top() + 2;
                    callStack.pop();
                    //std::cout << "0x00EE: return from subroutine" << std::endl;
                    break;
                // call to address XXX
                default:
                    //std::cout << "0x0XXX: call to address XXX" << std::endl;
                    //std::cout << "Not used in modern VMs" << std::endl;
                    return false;
                    //break;
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
            //std::cout << "GOTO: " << std::hex << progCounter << std::endl;
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

            //std::cout << "CALL: " << std::hex << progCounter << std::endl;
            break;

        // 0x3RXX (skip next opcode if registers[R] == XX
        case 0x3:
            if (registers[getHexDigit2(opCode)] == getHexDigits3and4(opCode))
            {
                progCounter += 4;
                //std::cout << "0x3 (skip next)" << std::endl;
            }
            else
            {
                progCounter += 2;
                //std::cout << "0x3 (don't skip next)" << std::endl;
            }
            break;

        // 0x4RXX (skip next opCode if registers[R] != XX
        case 0x4:
            if (registers[getHexDigit2(opCode)] != getHexDigits3and4(opCode))
            {
                progCounter += 4;
                //std::cout << "0x4RXX (skip next)" << std::endl;
            }
            else
            {
                progCounter += 2;
                //std::cout << "0x4RXX (don't skip next)" << std::endl;
            }
            break;

        // 0x5RS0 (skip next opCode if registers[R] == registers[S]
        case 0x5:
            if (getHexDigit4(opCode) != 0)
            {
                //std::cout << "OpCode not implemented (" << std::hex << opCode << ")" << std::endl;
                return false;
            }

            if (registers[getHexDigit2(opCode)] == registers[getHexDigit3(opCode)])
            {
                progCounter += 4;
                //std::cout << "0x5RS0 (skip next)" << std::endl;
            }
            else
            {
                progCounter += 2;
                //std::cout << "0x5RS0 (don't skip next)" << std::endl;
            }
            break;

        // 0x6RXX (registers[R] = XX)
        case 0x6:
            registers[getHexDigit2(opCode)] = getHexDigits3and4(opCode);
            progCounter += 2;
            //std::cout << "0x6RXX: " << std::hex << opCode << std::endl;
            break;

        // 0x7RXX (registers[R] += XX)
        case 0x7:
            registers[getHexDigit2(opCode)] += getHexDigits3and4(opCode);
            progCounter += 2;
            //std::cout << "0x7RXX" << std::endl;
            break;

        // 0x8XXX
        case 0x8:
            switch (getHexDigit4(opCode))
            {
                // 0x8RS0 (registers[R] = registers[S])
                case 0x0:
                    registers[getHexDigit2(opCode)] = registers[getHexDigit3(opCode)];
                    progCounter += 2;
                    //std::cout << "0x8RS0" << std::hex << opCode << std::endl;
                    break;

                // 0x8RS1 (registers[R] |= registers[S])
                case 0x1:
                    registers[getHexDigit2(opCode)] |= registers[getHexDigit3(opCode)];
                    progCounter += 2;
                    //std::cout << "0x8RS1" << std::hex << opCode << std::endl;
                    break;

                // 0x8RS2 (registers[R] &= registers[S])
                case 0x2:
                    registers[getHexDigit2(opCode)] &= registers[getHexDigit3(opCode)];
                    progCounter += 2;
                    //std::cout << "0x8RS2" << std::hex << opCode << std::endl;
                    break;

                // 0x8RS3 (registers[R] ^= registers[S])
                case 0x3:
                    registers[getHexDigit2(opCode)] ^= registers[getHexDigit3(opCode)];
                    progCounter += 2;
                    //std::cout << "0x8RS3" << std::hex << opCode << std::endl;
                    break;

                // 0x8RS4 (registers[R] += registers[S], updates carry (registers[0xF]))
                case 0x4:
                {
                    unsigned char r = registers[getHexDigit2(opCode)];
                    unsigned char s = registers[getHexDigit3(opCode)];
                    registers[getHexDigit2(opCode)] += s;
                    // TODO: VVV is r + s implicitly cast to a larger type so this works?
                    registers[0xF] = r > 0xFF - s ? 1 : 0; // carry bit
                    progCounter += 2;
                    //std::cout << "0x8RS4" << std::hex << opCode << std::endl;
                    break;
                }

                // 0x8RS5 (registers[R] -= registers[S], updates carry (registers[0xF]) as borrow)
                case 0x5:
                {
                    unsigned char r = registers[getHexDigit2(opCode)];
                    unsigned char s = registers[getHexDigit3(opCode)];
                    registers[getHexDigit2(opCode)] -= s;
                    // VVV Not sure if this is correct VVV
                    registers[0xF] = s > r ? 0 : 1; // carry (borrow) bit
                    progCounter += 2;
                    //std::cout << "0x8RS5" << std::hex << opCode << std::endl;
                    break;
                }

                // 0x8RS6 (registers[R] = registers[S] >> 1, updates carry (registers[0xF] = LSB))
                case 0x6:
                {
                    unsigned char s = registers[getHexDigit3(opCode)];
                    registers[getHexDigit2(opCode)] = s >> 1;
                    registers[0xF] = s & 1;
                    progCounter += 2;
                    //std::cout << "0x8RS6" << std::hex << opCode << std::endl;
                    break;
                }

                // 0x8RS7 (registers[R] = registers[S] - registers[R], updates carry (registers[0xF]) as borrow)
                case 0x7:
                {
                    unsigned char r = registers[getHexDigit2(opCode)];
                    unsigned char s = registers[getHexDigit3(opCode)];
                    registers[getHexDigit2(opCode)] = s - r;
                    registers[0xF] = r > s ? 0 : 1; // carry (borrow) bit
                    progCounter += 2;
                    //std::cout << "0x8RS7" << std::hex << opCode << std::endl;
                    break;
                }

                // 0x8RSE (registers[R] = registers[S] << 1, updates carry (registers[0xF] = MSB))
                // TODO: This opCode is described at least three different ways in various sources,
                //       not sure which is correct
                case 0xE:
                {
                    unsigned char s = registers[getHexDigit3(opCode)];
                    registers[0xF] = s >> 7;
                    s <<= 1;
                    registers[getHexDigit2(opCode)] = s;
                    //registers[getHexDigit3(opCode)] = s;
                    progCounter += 2;
                    //std::cout << "0x8RSE" << std::hex << opCode << std::endl;
                    break;
                }

                default:
                    //std::cout << "OpCode not implemented (" << std::hex << opCode << ")" << std::endl;
                    return false;
            }
            break;

        // 0x9RS0 (skips next opCode if registers[R] != registers[S])
        case 0x9:
            if (getHexDigit4(opCode) != 0)
            {
                //std::cout << "OpCode not implemented (" << std::hex << opCode << ")" << std::endl;
                return false;
            }

            if (registers[getHexDigit2(opCode)] != registers[getHexDigit3(opCode)])
            {
                progCounter += 4;
                //std::cout << "0x9 (skip next)" << std::endl;
            }
            else
            {
                progCounter += 2;
                //std::cout << "0x9 (don't skip next)" << std::endl;
            }
            break;

        // 0xAXXX (index = XXX)
        case 0xA:
            index = getHexAddress(opCode);
            progCounter += 2;
            //std::cout << "0xAXXX" << std::endl;
            break;

        // 0xBXXX (pc = registers[0] + XXX)
        case 0xB:
            progCounter = registers[0] + getHexAddress(opCode);
            //std::cout << "0xBXXX" << std::endl;
            break;

        // 0xCRXX (registers[R] = rand() & XX)
        case 0xC:
            registers[getHexDigit2(opCode)] = (rand() % 256) & getHexDigits3and4(opCode);
            progCounter += 2;
            //std::cout << "0xCXXX" << std::endl;
            break;

        // 0xDXYH (draw an 8xH sprite at x = registers[X], y = registers[Y])
        case 0xD:
        {
            registers[0xF] = 0;
            unsigned char x = registers[getHexDigit2(opCode)];
            unsigned char y = registers[getHexDigit3(opCode)];
            if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT)
            {
                unsigned char h = getHexDigit4(opCode);
                int xByte = x / 8;
                int xBit = x % 8;
                unsigned short start = SCREEN_START + xByte + (y * (SCREEN_WIDTH >> 3));
                //std::cout << std::dec << "x: " << (int)x << " y: " << (int)y << " h: " << (int)h << std::hex << " start: " << start << " index: " << index <<  std::endl;
                if (xBit == 0) // Drawing to a single byte per line
                {
                    for (int i = 0; i < h && i + y < SCREEN_HEIGHT; ++i)
                    {
                        unsigned short loc = start + i * (SCREEN_WIDTH >> 3);
                        unsigned char temp = memory[loc];
                        memory[loc] ^= memory[index + i];
                        if (temp & memory[loc])
                        {
                            registers[0xF] |= 1;
                        }
                    }
                }
                else // Drawing to two bytes per line
                {
                    for (int i = 0; i < h && i + y < SCREEN_HEIGHT; ++i)
                    {
                        unsigned short loc = start + i * (SCREEN_WIDTH >> 3);
                        unsigned char temp1 = memory[loc];
                        unsigned char toWrite1 = memory[index + i] >> xBit;
                        memory[loc] ^= toWrite1;
                        if (temp1 & toWrite1)
                        {
                            registers[0xF] |= 1;
                        }
                        if (xByte < 7)
                        {
                            unsigned char temp2 = memory[loc + 1];
                            unsigned char toWrite2 = memory[index + i] << (8 - xBit);
                            memory[loc + 1] ^= toWrite2;
                            if (temp2 & toWrite2)
                            {
                                registers[0xF] |= 1;
                            }
                        }
                    }
                }
            }
            progCounter += 2;
            //std::cout << "0xDXYH - Draw: " << std::hex << opCode << std::endl;
            break;
        }

        // 0xEXXX
        case 0xE:
            switch (getHexDigits3and4(opCode))
            {
                // 0xER9E (skip next opCode if key[registers[R]])
                case 0x9E:
                    if (keypad[registers[getHexDigit2(opCode)]])
                    {
                        progCounter += 4;
                    }
                    else
                    {
                        progCounter += 2;
                    }
                    break;

                // 0xERA1 (skip next opCode if !key[registers[R]])
                case 0xA1:
                    if (!keypad[registers[getHexDigit2(opCode)]])
                    {
                        progCounter += 4;
                    }
                    else
                    {
                        progCounter += 2;
                    }
                    break;

                // opCode is not implemented, so crash already
                default:
                    //std::cout << "OpCode not implemented (" << std::hex << opCode << ")" << std::endl;
                    return false;
            }
            break;

        // 0xFRXX
        case 0xF:
            switch (getHexDigits3and4(opCode))
            {
                // 0xFR07 (registers[R] = delayInterruptTimer)
                case 0x07:
                    registers[getHexDigit2(opCode)] = delayInterruptTimer;
                    progCounter += 2;
                    //std::cout << "0xFR07" << std::endl;
                    break;

                // FIXME: need better way to get keypress
                // 0xFR0A (execution waits for keypress, stored in registers[R])
                case 0x0A:
                    //std::cout << "0xFR0A: waiting for keypress..." << std::endl;
                    unsigned char tempChar;
                    do
                    {
                        tempChar = static_cast<unsigned char>(getchar());
                    } while (!isxdigit(tempChar));
                    registers[getHexDigit2(opCode)] = tempChar;
                    progCounter += 2;
                    //std::cout << "0xFR0A" << std::endl;
                    break;

                // 0xFR15 (delayInterruptTimer = registers[R])
                case 0x15:
                    delayInterruptTimer = registers[getHexDigit2(opCode)];
                    progCounter += 2;
                    //std::cout << "0xFR15 - Delay: " << std::hex << (int)delayInterruptTimer << std::endl;
                    break;

                // 0xFR18 (soundInterruptTimer = registers[R])
                case 0x18:
                    soundInterruptTimer = registers[getHexDigit2(opCode)];
                    progCounter += 2;
                    //std::cout << "0xFR18 - Sound: " << std::hex << (int)soundInterruptTimer << std::endl;
                    break;

                // 0xFR1E (index += registers[R])
                case 0x1E:
                    index += registers[getHexDigit2(opCode)];
                    progCounter += 2;
                    //std::cout << "0xFR1E: Index += Reg[]" << std::endl;
                    break;

                // 0xFR29 (set index to address of sprite for character in registers[R])
                case 0x29:
                    // TODO (?): check if index is legal
                    index = registers[getHexDigit2(opCode)] * BYTES_PER_FONT_CHAR;
                    progCounter += 2;
                    //std::cout << "0xFR29: index = font sprite address" << std::endl;
                    break;

                // 0xFR33 (binary-coded decimal of registers[R] stored in index, +1, +2)
                case 0x33:
                {
                    unsigned char tempNum = registers[getHexDigit2(opCode)];
                    memory[index] = tempNum / 100;
                    memory[index + 1] = tempNum / 10 % 10;
                    memory[index + 2] = tempNum % 10;
                    progCounter += 2;
                    //std::cout << "0xFR33: store decimal in 3 binary addrs" << std::endl;
                    break;
                }

                // 0xFR55 (registers[0 to R] are dumped to memory starting at index)
                case 0x55:
                {
                    int lastRegister = getHexDigit2(opCode);
                    // FIXME: this causes segfaults, may just need other opCodes implemented
                    // maybe good now ?
                    for (int i = 0; i <= lastRegister; ++i)
                    {
                        memory[index + i] = registers[i];
                    }
                    progCounter += 2;
                    //std::cout << "0xFR55: Write regs[0-R] at index" << std::endl;
                    break;
                }

                // 0xFR65 (memory starting at index copied to registers[0 to R])
                case 0x65:
                {
                    int lastRegister = getHexDigit2(opCode);
                    // FIXME: this causes segfaults, may just need other opCodes implemented
                    // maybe good now ?
                    for (int i = 0; i <= lastRegister; ++i)
                    {
                        registers[i] = memory[index + i];
                    }
                    progCounter += 2;
                    //std::cout << "0xFR55: Write index to regs[0-R]" << std::endl;
                    break;
                }

                // opCode is not implemented, so crash already
                default:
                    //std::cout << "OpCode not implemented (" << std::hex << opCode << ")" << std::endl;
                    return false;
            }
            break;

        default:
        {
            /* FIXME: this should stop execution once all opCodes are implemented
               just a catch for non-implemented opCodes */
            //progCounter += 2;
            return false;
            //std::cout << "default" << std::endl;
        }
    }

    //std::cout << "PC: " << std::hex << progCounter << std::endl;
    return true;
}

// TODO: Combine with decode() ?, these should be 60Hz regardless of main loop
// Execute the decoded operation
bool chip8::execute()
{
    if (soundInterruptTimer > 0)
    {
        /*
        if (soundInterruptTimer > 1)
            beep();
        */
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

