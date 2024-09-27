#include "Screen.hpp"

#include <windows.h>

#include <iostream>

int main()
{
    Screen screen;
    if (!screen.Initialize()) return EXIT_FAILURE;

    while (screen.Update())
    {
        screen.Clear();
        screen.Draw();
    }

    return EXIT_SUCCESS;
}
