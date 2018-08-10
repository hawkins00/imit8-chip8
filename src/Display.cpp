/**
 * Copyright (c) 2018 Chris Kim & Matt Hawkins
 * This program is licensed under the "GPLv3 License"
 * Please see the file License.md in the source
 * distribution of this software for license terms.
 */

 /*
 * Display
 * Class to Display output to the screen.
 */

#include "Display.h"

Display::
Display(unsigned char * screen, unsigned short height, unsigned short width)
{
    setHeight(height);
    setWidth(width);
    clearFrame();
    Display::screen = screen;
    std::clog << "Display was initialized with height: " << height << " and width: " << width << std::endl;
}


void Display::
drawDisplay()
{
    for(int row = 0; row < getHeight(); ++row)
    {
        for(int columnOf8 = 0; columnOf8 < getWidth() / 8; ++columnOf8)
        {
            printChar(screen[row * 8 + columnOf8]);
            if((columnOf8+1) % 8 == 0)
            {
                frame += '\n';
            }
        }
    }

    std::cout << frame << std::endl;
    std::flush(std::cout);
    clearFrame();
}

void Display::
clearScreen()
{
    #ifdef WINDOWS
        // assuming windows
        std::system("cls");
    #else
        // assuming linux
        std::system("clear");
    #endif
}

void Display::
printChar(unsigned char toPrint)
{
    for(int i = 0; i < 8; ++i)
    {
        if(toPrint & (0x80 >> i))
        {
            frame += "▀"; //█▀
        }
        else
        {
            frame += " ";
        }
    }
}

int Display::
getHeight() const
{
    return height;
}

void Display::
setHeight(int height)
{
    Display::height = height;
}

int Display::
getWidth() const
{
    return width;
}

void Display::
setWidth(int width)
{
    Display::width = width;
}

void Display::clearFrame()
{
    frame = "";
}
