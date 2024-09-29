#include <windows.h>

#include <iostream>
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

private:
    enum class Color
    {
        Black = RGB(0, 0, 0),
        Gray = RGB(192, 192, 192),
        Green = RGB(19, 161, 14),
    };

    class Line
    {
    public:
        Line(int column);
        ~Line();

        void Update(Screen* screen);
        void Draw(Screen* screen);

    private:
        int m_column;
        int m_row;
        std::string m_data;
    };

    static const int ScreenWidth = 1920;
    static const int ScreenHeight = 1080;
    static const int MaxColumn = 96;
    static const int MaxRow = 54;
    static const int FontHeight = 20;
    static const int FontWidth = FontHeight / 2;
    const std::string Font = "Cascadia Mono SemiBold";
    static const int WaitTime = 1000 /*ms*/ / 20 /*fps*/;

    static bool m_loop;
    HWND m_workerW;
    std::string m_path;
    HDC m_hdc;
    std::vector<Line*> m_lines;
    std::vector<Line*> m_delete;
    std::vector<Line*> m_insert;

    bool setWorkerW();
    bool getWallpaper();
    bool setWallpaper(const std::string path);
    bool setFont();
    void drawText(const int x, const int y, const Color color, const std::string text);
    void drawRectangle(const int x, const int y, const int w, const int h, const Color color);
};
