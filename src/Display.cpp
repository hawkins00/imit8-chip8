/*
 * Display
 * Class to Display output to the screen.
 */

#include "Display.h"

Display::Display(unsigned char * screen, unsigned short height, unsigned short width)
{
    setHeight(height);
    setWidth(width);
    Display::screen = screen;
    std::cout << "Display was initialized with height: " << height << " and a width: " << width << " value." << std::endl;
}


void Display::drawDisplay()
{
    std::cout << std::endl;
    for(int row = 0; row < getHeight(); ++row)
    {
        for(int columnOf8 = 0; columnOf8 < getWidth() / 8; ++columnOf8)
        {
            printChar(screen[row * 8 + columnOf8]);
            if((columnOf8+1) % 8 == 0)
            {
                std::cout << std::endl;
            }
        }
    }
}

void Display::clearScreen()
{
    #ifdef WINDOWS
        //std::cout << "assuming windows";
        std::system("cls");
    #else
        //std::cout << "assuming linux";
        std::system("clear");
    #endif
}

void Display::printChar(unsigned char toPrint)
{
    for(int i = 0; i < 8; ++i)
    {
        if(toPrint & (128 >> i))
        {
            std::cout << "█"; //█▀
        }
        else
        {
            std::cout << " ";
        }
    }
}

int Display::getHeight() const
{
    return height;
}

void Display::setHeight(int height)
{
    Display::height = height;
}

int Display::getWidth() const
{
    return width;
}

void Display::setWidth(int width)
{
    Display::width = width;
}
