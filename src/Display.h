/**
 * Copyright (c) Chris Kim & Matt Hawkins
 * This program is licensed under the "GPLv3 License"
 * Please see the file License.md in the source
 * distribution of this software for license terms.
 */

/*
 * Display
 * Class to Display output to the screen.
 */

#ifndef IMIT8_CHIP8_DISPLAY_H
#define IMIT8_CHIP8_DISPLAY_H

#include <iostream>

class Display
{
    public:
        Display(unsigned char * screen, unsigned short height = 32, unsigned short width = 64);
        void drawDisplay();
        static void clearScreen();

    private:
        int height;
        int width;
        unsigned char* screen;
        std::string frame;

        int getHeight() const;
        void setHeight(int height);
        int getWidth() const;
        void setWidth(int width);
        void printChar(unsigned char toPrint);
        void clearFrame();
};

#endif //IMIT8_CHIP8_DISPLAY_H
