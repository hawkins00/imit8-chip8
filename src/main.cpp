#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <thread>
#include "chip8.h"
#include "Display.h"
#include "logWriter.h"

using namespace std::chrono;

#define USECONDS_PER_FRAME (1000000 / 60)
#define OPCODES_PER_FRAME 10

int main(int argc, char* argv[])
{
    Display::clearScreen();
    logWriter LogWriter("DEBUG_LOG.txt", logWriter::logLevel::INFO);

    // check for correct num of args
    if (argc != 2)
    {
        LogWriter.log(logWriter::logLevel::ERROR, "No input program file provided. Exiting.");
        std::cerr << "ERROR: No input program file provided." << std::endl;
        std::cerr << "Usage: imit8-chip8 dir/filename.ext" << std::endl;
        exit(1);
    }

    chip8 cpu0;
    Display screen(cpu0.getScreen()); // create display and give access to vram

    // load the ROM file
    if (!cpu0.loadFile(argv[1]))
    {
        std::string loadFileFail = "ROM file (";
        loadFileFail += argv[1];
        loadFileFail += ") could not be loaded. Exiting.";
        LogWriter.log(logWriter::logLevel::ERROR, loadFileFail);
        exit(2);
    }
    else
    {
        std::string loadFile = "ROM file (";
        loadFile += argv[1];
        loadFile += ") successfully loaded.";
        LogWriter.log(logWriter::logLevel::INFO, loadFile);
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
        long int diff = frameEnd.count() - frameStart.count();
        std::this_thread::sleep_for(microseconds(USECONDS_PER_FRAME - diff));
    }

    LogWriter.log(logWriter::logLevel::INFO, "Program loop exited. Shutting down.");

    return 0;
}