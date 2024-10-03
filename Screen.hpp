#include <windows.h>

#include <iostream>
#include <list>
#include <string>
#include <vector>

class Screen
{
public:
    Screen();
    ~Screen();

    bool Initialize();
    bool Update();
    void Draw();
    void Clear();
    void Flip();

private:
    enum class Color
    {
        Black = RGB(0, 0, 0),
        Gray = RGB(192, 192, 192),
        Green = RGB(19, 161, 14),
    };

    struct Line
    {
        Line(int column)
        {
            Column = column;

            Length = 4 + rand() % 16;

            // 画面外に上部に初期位置をセット
            Row = -1 * (Length + 4 + rand() % 16);
        }

        ~Line() {};

        int Column;
        int Row;
        int Length;
    };

    static const int ScreenWidth = 1920 * 2;
    static const int ScreenHeight = 1080;
    static const int FontHeight = 20;
    static const int FontWidth = FontHeight / 2;
    static const int MaxColumn = ScreenWidth / (FontWidth * 2);
    static const int MaxRow = ScreenHeight / FontHeight;
    static const int WaitTime = 1000 /*ms*/ / 20 /*fps*/;

    static bool m_loop;
    HWND m_workerW;
    std::string m_path;
    HDC m_buffer;
    HBITMAP m_bitmap;
    std::list<Line> m_lines;
    std::vector<std::string> m_strings;

    bool setWorkerW();
    bool getWallpaper();
    bool setWallpaper(const std::string path);
};
