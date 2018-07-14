#include <iostream>
#include "chip8.h"
#include "logWriter.h"

int main(int argc, char *argv[])
{
    logWriter LogWriter("newlogfile.txt");

    if (argc != 2)
    {
        LogWriter.log("Usage: imit8-chip8 filename.ext");
        std::cout << LogWriter.getOutputFileName() << std::endl;
        exit(1);
    }

    // TODO: write log statements for stuff that happens
    chip8 cpu0;
    cpu0.load(argv[1]);
    bool isRunning = true;

    while (isRunning)
    {
        // TODO: run one cycle
        // TODO: update screen
        // TODO: read key state
        isRunning = false;
    }

    return 0;
}