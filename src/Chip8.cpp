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

#include "Chip8.h"
#include "LogWriter.h"

Chip8::
Chip8()
{
    //logWriter.log(logWriter::logLevel::INFO, "Creating CHIP-8 CPU...");
    init();
}

bool Chip8::
init()
{
    std::cout << "Initializing CPU..." << std::endl;

    srand(static_cast<unsigned int>(time(nullptr)));

    for (unsigned char& i : registers)
    {
        i = 0;
    }

    for (unsigned char& i : graphicsBuffer)
    {
        i = 0;
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
    isDirty = false;

    //std::cout << "Done initializing CPU..." << std::endl;
    return true;
}


// Load game file into memory at 0x200 (decimal 512)
bool Chip8::
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
bool Chip8::
loadFontSet()
{
    std::copy_n(font, FONT_SIZE, memory);
    return true;
}

bool Chip8::
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
unsigned char* Chip8::
getScreen()
{
    return graphicsBuffer;
}

// Run the next cycle
bool Chip8::
runCycle()
{
    isDirty = false;
    return fetch() && decodeAndExecute();
}

// Fetch the next opCode
bool Chip8::
fetch()
{
    opCode = memory[progCounter] << 8 | memory[progCounter + 1];
    //std::cout << std::hex << (int)progCounter << " ops: " << (int)memory[progCounter] << " " << (int)memory[progCounter + 1] << std::endl;
    //std::cout << std::hex << "ops (F2C): " << (int)memory[0xF2C] << " " << (int)memory[0xF2D] << std::endl;
    //std::cout << "Fetching - PC: " << progCounter << " opCode: " << std::hex << opCode << std::endl;
    return true;
}

// Decode the fetched opCode and execute it
bool Chip8::
decodeAndExecute()
{
    switch (getHexDigit1(opCode))
    {
        // 0x0XXX ()
        case 0x0:
            switch (getHexAddress(opCode))
            {
                // 0x00E0 (clear the screen)
                case 0x0E0:
                    for (unsigned char& i : graphicsBuffer)
                    {
                        i = 0;
                    }
                    progCounter += 2;
                    isDirty = true;
                    //std::cout << "0x00E0: clear screen" << std::endl;
                    break;

                // 0x00EE (return from subroutine)
                case 0x0EE:
                    if (callStack.empty())
                    {
                        //std::cout << "0x00EE: call stack is empty" << std::endl;
                        exit(3);
                    }
                    //std::cout << "0x00EE: progCounter: " << std::hex << progCounter << std::endl;
                    //std::cout << "      : StackTop: " << std::hex << callStack.top() << std::endl;
                    progCounter = callStack.top() + 2;
                    callStack.pop();
                    //std::cout << "0x00EE: return from subroutine, PC: " << std::hex << progCounter << std::endl;
                    break;

                // call to address XXX
                default:
                    std::cout << "0x0XXX: call to address XXX" << std::endl;
                    std::cout << "Not used in modern VMs" << std::endl;
                    return false;
            }
            break;

        // 0x1XXX (goto)
        case 0x1:
        {
            unsigned short prevProgramCounter = progCounter;
            progCounter = getHexAddress(opCode);
            // if in a single-instruction goto loop, may as well end computation
            if (progCounter == prevProgramCounter)
            {
                std::cerr << "Execution ended due to GOTO loop (" << std::hex << opCode << ")" << std::endl;
                return false;
            }
            //std::cout << "GOTO: " << std::hex << progCounter << std::endl;
            break;
        }

        // 0x2XXX (subroutine call)
        case 0x2:
            callStack.push(progCounter);
            progCounter = getHexAddress(opCode);

            // stack overflow
            if (callStack.size() > STACK_DEPTH)
            {
                std::cout << "Stack Overflow: " << std::hex << progCounter << std::endl;
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
                std::cout << "OpCode not implemented (" << std::hex << opCode << ")" << std::endl;
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
                    registers[0xF] = s > r ? 0 : 1; // carry (borrow) bit
                    progCounter += 2;
                    //std::cout << "0x8RS5" << std::hex << opCode << std::endl;
                    break;
                }

                // 0x8RX6 (registers[R] >>= 1, registers[0xF] = LSB)
                // Wikipedia info on this opCode is incorrect
                case 0x6:
                {
                    unsigned char r = registers[getHexDigit2(opCode)];
                    registers[0xF] = r & 0x1;
                    registers[getHexDigit2(opCode)] = r >> 1;
                    progCounter += 2;
                    //std::cout << "0x8RX6" << std::hex << opCode << std::endl;
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

                // 0x8RXE (registers[R] <<= 1, registers[0xF] = MSB)
                // Wikipedia info on this opCode is incorrect
                case 0xE:
                {
                    unsigned char r = registers[getHexDigit2(opCode)];
                    registers[0xF] = r >> 7;
                    registers[getHexDigit2(opCode)] = r << 1;
                    progCounter += 2;
                    //std::cout << "0x8RSE" << std::hex << opCode << std::endl;
                    break;
                }

                default:
                    std::cout << "OpCode not implemented (" << std::hex << opCode << ")" << std::endl;
                    return false;
            }
            break;

        // 0x9RS0 (skips next opCode if registers[R] != registers[S])
        case 0x9:
            if (getHexDigit4(opCode) != 0)
            {
                std::cout << "OpCode not implemented (" << std::hex << opCode << ")" << std::endl;
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

        // FIXME: some ROMs only work on systems where vram is not part of the 4kb, so do that
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
                unsigned short start = xByte + y * SCREEN_WIDTH_SIZE;
                //std::cout << std::dec << "x: " << (int)x << " y: " << (int)y << " h: " << (int)h << std::hex << " start: " << start << " index: " << index <<  std::endl;
                if (xBit == 0) // Drawing to a single byte per line
                {
                    for (int i = 0; i < h; ++i)
                    {
                        unsigned short loc = (start + i * SCREEN_WIDTH_SIZE) % SCREEN_SIZE;
                        /*
                        unsigned short loc = start + i * (SCREEN_WIDTH >> 3);
                        if (loc > MEMORY_SIZE)
                        {
                            loc -= ((SCREEN_WIDTH * SCREEN_HEIGHT) >> 6);
                        }
                         */
                        unsigned char temp = graphicsBuffer[loc];
                        graphicsBuffer[loc] ^= memory[index + i];
                        if (temp & ~graphicsBuffer[loc])
                        {
                            registers[0xF] = 1;
                        }
                    }
                }
                else // Drawing to two bytes per line
                {
                    for (int i = 0; i < h; ++i)
                    {
                        unsigned short loc = (start + i * SCREEN_WIDTH_SIZE) % SCREEN_SIZE;
                        unsigned char temp1 = graphicsBuffer[loc];
                        unsigned char toWrite1 = memory[index + i] >> xBit;
                        graphicsBuffer[loc] ^= toWrite1;
                        if (temp1 & ~graphicsBuffer[loc])
                        {
                            registers[0xF] = 1;
                        }
                        if (xByte < 7) // Does not wrap horizontally
                        {
                            unsigned char temp2 = graphicsBuffer[loc + 1];
                            unsigned char toWrite2 = memory[index + i] << (8 - xBit);
                            graphicsBuffer[loc + 1] ^= toWrite2;
                            if (temp2 & ~graphicsBuffer[loc + 1])
                            {
                                registers[0xF] = 1;
                            }
                        }
                        else // Does wrap horizontally
                        {
                            unsigned char temp2 = graphicsBuffer[loc + 1 - 8];
                            unsigned char toWrite2 = memory[index + i] << (8 - xBit);
                            graphicsBuffer[loc + 1 - 8] ^= toWrite2;
                            if (temp2 & ~graphicsBuffer[loc + 1 - 8])
                            {
                                registers[0xF] = 1;
                            }
                        }
                    }
                }
            }
            progCounter += 2;
            isDirty = true;
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
                    std::cout << "OpCode not implemented (" << std::hex << opCode << ")" << std::endl;
                    //std::cout << "PC: " << std::hex << progCounter << std::endl;
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
                    for (int i = 0; i <= lastRegister; ++i)
                    {
                        memory[index + i] = registers[i];
                    }
                    // some sources say to do the next line, others say don't
                    // index += lastRegister + 1;
                    progCounter += 2;
                    //std::cout << "0xFR55: Write regs[0-R] at index" << std::endl;
                    break;
                }

                // 0xFR65 (memory starting at index copied to registers[0 to R])
                case 0x65:
                {
                    int lastRegister = getHexDigit2(opCode);
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
                    std::cout << "OpCode not implemented (" << std::hex << opCode << ")" << std::endl;
                    return false;
            }
            break;

        default:
        {
            std::cout << "OpCode not implemented (" << std::hex << opCode << ")" << std::endl;
            return false;
        }
    }

    //std::cout << "PC: " << std::hex << progCounter << " OpCode: " << opCode << std::endl;
    return true;
}

// Update timers @ 60 Hz
bool Chip8::
updateTimers()
{
    if (soundInterruptTimer > 0)
    {
        if (soundInterruptTimer > 1)
            std::cout << "beeping..." << std::endl;
        --soundInterruptTimer;
    }
    if (delayInterruptTimer > 0)
    {
        --delayInterruptTimer;
    }

    return true;
}

unsigned char Chip8::
getHexDigit1(unsigned short hexShort)
{
    return hexShort >> 12;
}

unsigned char Chip8::
getHexDigit2(unsigned short hexShort)
{
    return (hexShort >> 8) & 0xF;
}

unsigned char Chip8::
getHexDigit3(unsigned short hexShort)
{
    return (hexShort >> 4) & 0xF;
}

unsigned char Chip8::
getHexDigit4(unsigned short hexShort)
{
    return hexShort & 0x000F;
}

unsigned char Chip8::
getHexDigits3and4(unsigned short hexShort)
{
    return hexShort & 0x00FF;
}

unsigned short Chip8::
getHexAddress(unsigned short hexShort)
{
    return hexShort & 0x0FFF;
}

bool Chip8::
isDirtyScreen()
{
    return isDirty;
}

