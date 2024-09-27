#include <windows.h>

#include <functional>
#include <iostream>
#include <string>

constexpr int Width = 1920;
constexpr int Height = 1080;
constexpr COLORREF Black = RGB(0, 0, 0);
constexpr COLORREF Green = RGB(19, 161, 14);

HWND getWorkerW()
{
    // デスクトップ画面を管理するウィンドウを取得
    const HWND progman = GetShellWindow();
    std::cout << "Progman: " << progman << std::endl;

    // メッセージを送ってWorkerWを生成させる
    // 詳細：https://www.codeproject.com/Articles/856020/Draw-Behind-Desktop-Icons-in-Windows-plus
    SendMessageTimeout(progman, 0x052C, NULL, NULL, SMTO_NORMAL, 1000, NULL);

    // WorkerWは複数ある
    // アイコン裏の描画はデスクトップ管理ウィンドウの次のWorkerW
    HWND workerW = GetNextWindow(progman, GW_HWNDPREV);

    std::cout << "WorkerW: ";

    if (workerW == NULL) std::cout << "NULL" << std::endl;
    else std::cout << workerW << std::endl;

    return workerW;
}

std::string getWallpaper()
{
    char path[MAX_PATH]{};

    std::cout << "Wallpaper: ";

    // デスクトップの壁紙を取得
    if (SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, path, NULL))
    {
        std::cout << path << std::endl;
        return path;
    }
    else
    {
        std::cout << "NULL" << std::endl;
        return "";
    }
}

void setWallpaper(const std::string path)
{
    std::cout << "Set Wallpaper: ";

    // デスクトップの壁紙を設定
    if (SystemParametersInfo(SPI_SETDESKWALLPAPER, NULL, (PVOID)path.c_str(),
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE)) std::cout << "OK" << std::endl;
    else std::cout << "Error" << std::endl;
}

void setFont(const HDC hdc)
{
    const HFONT font = CreateFont(20, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
                                  ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                                  DRAFT_QUALITY, DEFAULT_PITCH, "Cascadia Mono SemiBold");
    SelectObject(hdc, font);
    DeleteObject(font);
}

void drawBack(const HDC hdc)
{
    const HBRUSH brush = CreateSolidBrush(Black);
    const HBRUSH old = (HBRUSH)SelectObject(hdc, brush);

    PatBlt(hdc, 0, 0, Width, Height, PATCOPY);

    SelectObject(hdc, old);
    DeleteObject(brush);
}

void drawText(const HDC hdc, int x, int y, const COLORREF color, const std::string text)
{
    // 文字の後ろは塗りつぶさない
    SetBkMode(hdc, TRANSPARENT);

    SetTextColor(hdc, color);

    TextOut(hdc, x, y, text.c_str(), strlen(text.c_str()));
}

int main()
{
    const HWND workerW = getWorkerW();
    if (workerW == NULL) return -1;

    const std::string path = getWallpaper();
    if (path == "") return -1;

    const HDC hdc = GetDC(workerW);
    const HDC back = CreateCompatibleDC(hdc);
    const HBITMAP bitmap = CreateCompatibleBitmap(hdc, Width, Height);

    SelectObject(back, bitmap);
    SelectObject(back, GetStockObject(DC_BRUSH));

    setFont(hdc);

    for (int i = 0; i < 5; i++)
    {
        drawBack(hdc);
        drawText(hdc, 0, 20 * i, Green, "Text");

        std::cin.get();
    }

    DeleteDC(back);
    DeleteObject(back);
    ReleaseDC(workerW, hdc);

    setWallpaper(path);

    return 0;
}
