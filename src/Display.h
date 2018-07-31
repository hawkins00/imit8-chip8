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
        unsigned char * screen;

        int getHeight() const;
        void setHeight(int height);
        int getWidth() const;
        void setWidth(int width);
        void printChar(unsigned char toPrint);
};

#endif //IMIT8_CHIP8_DISPLAY_H
