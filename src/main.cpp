#include <algorithm>
#include <iostream>
#include <unistd.h>
#include "chip8.h"
#include "Display.h"
#include "logWriter.h"

int main(int argc, char* argv[])
{
    // TODO: Figure out why default constructor does not work.
    logWriter LogWriter("DEBUG_LOG.txt", logWriter::logLevel::INFO);

    if (argc != 2)
    {
        LogWriter.log(logWriter::logLevel::ERROR, "No input program file provided.  Exiting.");
        std::cout << "ERROR:  No input program file provided." << std::endl;
        std::cout << "Usage:  imit8-chip8 filename.ext" << std::endl;
        exit(1);
    }

    // TODO: write log statements for stuff that happens
    chip8 cpu0;
    Display screen(cpu0.getScreen());
    if (!cpu0.loadFile(argv[1]))
    {
        return 1;
    }

    bool isRunning = true;
    //std::chrono::milliseconds ms = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    clock_t start;
    clock_t end;
    while (isRunning)
    {
        start = clock();
        for (int i = 0; i < 5; ++i)
        {
            isRunning &= cpu0.runCycle();
        }
        screen.drawDisplay();
        end = clock();
        __useconds_t diff = end - start;
        //std::cout << diff << std::endl;
        usleep(std::max(16667 - (int)diff, 0));
        // TODO: read key state
        //getchar();
    }

    return 0;
}