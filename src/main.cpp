/**
 * Copyright (c) Chris Kim & Matt Hawkins
 * This program is licensed under the "GPLv3 License"
 * Please see the file License.md in the source
 * distribution of this software for license terms.
 */

#include <thread>
#include "Chip8.h"
#include "Display.h"
#include "LogWriter.h"

using namespace std::chrono;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "ERROR: No input program file provided." << std::endl;
        std::cerr << "Usage: imit8-chip8 dir/filename.ext" << std::endl;
        exit(1);
    }

    LogWriter logWriter;
    Chip8 cpu0(&logWriter);
    Display screen(cpu0.getScreen(), &logWriter); // create display and give access to vram

    // load the ROM file
    if (!cpu0.loadFile(argv[1]))
    {
        std::string loadFileFail = "ROM file (";
        loadFileFail += argv[1];
        loadFileFail += ") could not be loaded. Exiting.\n";
        std::cout << loadFileFail << std::endl;
        exit(2);
    }

    Display::clearScreen();
    bool isRunning = true;

    // main execution loop
    do
    {
        microseconds frameStart = duration_cast<microseconds>(system_clock::now().time_since_epoch());
        bool toDraw = false;

        // run one frame's worth of opCodes
        for (int i = 0; i < OPCODES_PER_FRAME && isRunning; ++i)
        {
            isRunning = cpu0.runCycle();
            toDraw |= cpu0.isDirtyScreen();
        }

        // update screen, if necessary
        if (toDraw)
        {
            screen.drawDisplay();
        }

        cpu0.updateTimers();

        // sleep to ensure screen updates occur at 60 Hz
        microseconds frameEnd = duration_cast<microseconds>(system_clock::now().time_since_epoch());
        microseconds diff = microseconds(USECONDS_PER_FRAME) - (frameEnd - frameStart);
        std::this_thread::sleep_for(diff);
    } while (isRunning);

    logWriter.log(LogWriter::LogLevel::INFO, "Program loop exited normally. Shutting down.\n");

    return 0;
}