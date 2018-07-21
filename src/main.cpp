#include <iostream>
#include "chip8.h"
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
    if (!cpu0.loadFile(argv[1]))
    {
        return 1;
    }

    //bool isRunning = true;

    while (cpu0.nextCycle())
    {
        // TODO: run one cycle
        // TODO: update screen
        // TODO: read key state
    }

    return 0;
}