/**
 * Copyright (c) 2018 Chris Kim & Matt Hawkins
 * This program is licensed under the "GPLv3 License"
 * Please see the file License.md in the source
 * distribution of this software for license terms.
 */

#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <thread>
#include "Chip8.h"
#include "Display.h"
#include "LogWriter.h"

using namespace std::chrono;
#define OPCODES_PER_SECOND 600
#define FRAMES_PER_SECOND 60
#define OPCODES_PER_FRAME (OPCODES_PER_SECOND / FRAMES_PER_SECOND)
const microseconds USECONDS_PER_FRAME = microseconds(1000000 / FRAMES_PER_SECOND);

int main(int argc, char* argv[])
{
    Display::clearScreen();
    LogWriter logWriter("DEBUG_LOG.txt", LogWriter::LogLevel::INFO);

    // check for correct num of args
    if (argc != 2)
    {
        logWriter.log(LogWriter::LogLevel::ERROR, "No input program file provided. Exiting.\n");
        std::cerr << "ERROR: No input program file provided." << std::endl;
        std::cerr << "Usage: imit8-chip8 dir/filename.ext" << std::endl;
        exit(1);
    }

    Chip8 cpu0(logWriter);
    Display screen(cpu0.getScreen()); // create display and give access to vram

    // load the ROM file
    if (!cpu0.loadFile(argv[1]))
    {
        std::string loadFileFail = "ROM file (";
        loadFileFail += argv[1];
        loadFileFail += ") could not be loaded. Exiting.\n";
        std::cout << loadFileFail << std::endl;
        exit(2);
    }

    bool isRunning = true;
    // main execution loop
    while (isRunning)
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

        // update 60 Hz timers
        cpu0.updateTimers();

        // TODO: read key state

        // sleep to ensure screen updates occur at 60 Hz
        microseconds frameEnd = duration_cast<microseconds>(system_clock::now().time_since_epoch());
        microseconds diff = frameEnd - frameStart;
        std::this_thread::sleep_for(USECONDS_PER_FRAME - diff);
    }

    logWriter.log(LogWriter::LogLevel::INFO, "Program loop exited normally. Shutting down.\n");

    return 0;
}