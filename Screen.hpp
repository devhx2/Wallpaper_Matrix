#include <windows.h>

#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

enum class Color
{
    Black = RGB(0, 0, 0),
    Green = RGB(19, 161, 14),
};

class Screen
{
public:
    Screen();
    ~Screen();

    bool Initialize();

    void Write(const int x, const int y, const Color color, const std::string text) const;
    void Clear() const;

private:
    const int ScreenWidth = 1920;
    const int ScreenHeight = 1080;
    const int MaxColumn = 96;
    const int MaxRow = 54;
    const std::string Font = "Cascadia Mono SemiBold";

    HWND m_workerW;
    std::string m_path;
    HDC m_hdc;

    bool setWorkerW();
    bool getWallpaper();
    bool setWallpaper(const std::string path);
    bool setFont();
};
