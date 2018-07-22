/*
 * Display
 * Class to Display output to the screen.
 */

#include <iostream>
#include "Display.h"

Display::Display()
{
    std::cout << "Display was initialized without a height and a width value.  Using the Chip-8 default (64x32)." << std::endl;
    setHeight(64);
    setWidth(32);
}

Display::Display(int height, int width)
{
    std::cout << "Setting display to height of " << height << " and width of " << width << "." << std::endl;
    setHeight(height);
    setWidth(width);
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
