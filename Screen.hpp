#include <windows.h>

#include <iostream>
#include <string>

class Screen
{
public:
    Screen();
    ~Screen();

    bool Initialize();
    bool Update();
    void Draw();
    void Clear() const;

private:
    enum class Color
    {
        Black = RGB(0, 0, 0),
        Green = RGB(19, 161, 14),
    };

    const int ScreenWidth = 1920;
    const int ScreenHeight = 1080;
    const int MaxColumn = 96;
    const int MaxRow = 54;
    const std::string Font = "Cascadia Mono SemiBold";
    const int WaitTime = 1000 /*ms*/ / 20 /*fps*/;

    static bool m_loop;
    HWND m_workerW;
    std::string m_path;
    HDC m_hdc;

    bool setWorkerW();
    bool getWallpaper();
    bool setWallpaper(const std::string path);
    bool setFont();
    void writeText(const int x, const int y, const Color color, const std::string text) const;
};
