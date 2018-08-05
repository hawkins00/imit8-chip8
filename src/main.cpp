#include <algorithm>
#include <iostream>
#include <unistd.h>
#include "chip8.h"
#include "Display.h"
#include "logWriter.h"

int main(int argc, char* argv[])
{
    Display::clearScreen();
    logWriter LogWriter("DEBUG_LOG.txt", logWriter::logLevel::INFO);

    if (argc != 2)
    {
        LogWriter.log(logWriter::logLevel::ERROR, "No input program file provided. Exiting.");
        std::cout << "ERROR: No input program file provided." << std::endl;
        std::cout << "Usage: imit8-chip8 filename.ext" << std::endl;
        exit(1);
    }

    chip8 cpu0;
    Display screen(cpu0.getScreen());
    if (!cpu0.loadFile(argv[1]))
    {
        exit(2);
    }

    bool isRunning = true;
    clock_t start;
    while (isRunning)
    {
        start = clock();
        bool toDraw = false;
        for (int i = 0; i < 100 && isRunning; ++i)
        {
            isRunning = cpu0.runCycle();
            toDraw |= cpu0.isDirtyScreen();
        }
        if (toDraw)
        {
            screen.drawDisplay();
        }
        __useconds_t diff = clock() - start;
        usleep(std::max(166667 - static_cast<int>(diff), 0));
        cpu0.updateTimers();
        // TODO: read key state
    }

    return 0;
}