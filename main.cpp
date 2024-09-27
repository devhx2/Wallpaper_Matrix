#include "Screen.hpp"

#include <windows.h>

#include <iostream>

const int WaitTime = 1000 /*ms*/ / 20 /*fps*/;
static bool g_loop = true;

int main()
{
    std::cout << "Ctrl Handle: ";
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)[](DWORD event) -> BOOL
    {
        switch (event)
        {
            // イベントはexe実行のみ機能
            // VisualStudioからの実行では機能しない
            case CTRL_C_EVENT:
            case CTRL_CLOSE_EVENT:
            case CTRL_SHUTDOWN_EVENT:
                g_loop = false;
                return TRUE;
            default:
                return FALSE;
        }
    }, TRUE))
    {
        std::cout << "Error" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Set" << std::endl;

    Screen screen;
    if (!screen.Initialize()) return EXIT_FAILURE;

    int count = 0;
    while (g_loop)
    {
        screen.Clear();
        screen.Write(5,  0 + count * 20 - 100, Color::Green, "p");
        screen.Write(5, 20 + count * 20 - 100, Color::Green, "q");
        screen.Write(5, 40 + count * 20 - 100, Color::Green, "r");
        screen.Write(5, 60 + count * 20 - 100, Color::Green, "s");
        screen.Write(5, 80 + count * 20 - 100, Color::Green, "t");
        count++;
        Sleep(WaitTime);
    }

    return EXIT_SUCCESS;
}
