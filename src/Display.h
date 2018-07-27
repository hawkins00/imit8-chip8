/*
 * Display
 * Class to Display output to the screen.
 */

#ifndef IMIT8_CHIP8_DISPLAY_H
#define IMIT8_CHIP8_DISPLAY_H

class Display
{
    public:
        Display();
        void drawDisplay();

    private:


        int height;
        int width;
        char displayBuffer[256];
        int getHeight() const;

        void setHeight(int height);
        int getWidth() const;
        void setWidth(int width);


        void clearScreen();
        void printChar(unsigned char toPrint);
};

#endif //IMIT8_CHIP8_DISPLAY_H
