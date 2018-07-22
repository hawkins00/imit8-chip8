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
        Display(int height, int width);

        void drawDisplay();



    private:
        int height;
        int width;

        char displayBuffer[256];


        int getHeight() const;
        void setHeight(int height);

        int getWidth() const;
        void setWidth(int width);


};

#endif //IMIT8_CHIP8_DISPLAY_H
