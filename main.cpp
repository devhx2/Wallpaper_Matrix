#include <windows.h>

#include <functional>
#include <iostream>
#include <string>

constexpr int ScreenWidth = 1920;
constexpr int ScreenHeight = 1080;
constexpr COLORREF Black = RGB(0, 0, 0);
constexpr COLORREF Green = RGB(19, 161, 14);
constexpr int MaxColumn = 96;
constexpr int MaxRow = 54;
const std::string Font = "Cascadia Mono SemiBold";

HWND g_workerW;
std::string g_path;
HDC g_hdc;
bool g_loop;

bool setWorkerW()
{
    // デスクトップ画面を管理するウィンドウを取得
    const HWND progman = GetShellWindow();

    // メッセージを送ってWorkerWを生成させる
    // 詳細：https://www.codeproject.com/Articles/856020/Draw-Behind-Desktop-Icons-in-Windows-plus
    SendMessageTimeout(progman, 0x052C, NULL, NULL, SMTO_NORMAL, 1000, NULL);

    // WorkerWは複数ある
    // アイコン裏の描画はデスクトップ管理ウィンドウの次のWorkerW
    g_workerW = GetNextWindow(progman, GW_HWNDPREV);

    return g_workerW != NULL;
}

bool getWallpaper()
{
    char path[MAX_PATH]{};

    // デスクトップの壁紙を取得
    if (SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, (PVOID)path, NULL))
    {
        g_path = path;
        return true;
    }
    return false;
}

bool setWallpaper(const std::string path)
{
    // デスクトップの壁紙を設定
    return SystemParametersInfo(SPI_SETDESKWALLPAPER, NULL, (PVOID)path.c_str(),
                                SPIF_UPDATEINIFILE | SPIF_SENDCHANGE) == TRUE;
}

bool setFont()
{
    const HFONT font = CreateFont(20, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
                                  ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                                  DRAFT_QUALITY, DEFAULT_PITCH, Font.c_str());
    SelectObject(g_hdc, font);
    DeleteObject(font);

    return true;
}

void clearScreen()
{
    const HBRUSH brush = CreateSolidBrush(Black);
    const HBRUSH old = (HBRUSH)SelectObject(g_hdc, brush);

    PatBlt(g_hdc, 0, 0, ScreenWidth, ScreenHeight, PATCOPY);

    SelectObject(g_hdc, old);
    DeleteObject(brush);
}

void drawText(const int x, const int y, const COLORREF color, const std::string text)
{
    // 文字の後ろは塗りつぶさない
    SetBkMode(g_hdc, TRANSPARENT);

    SetTextColor(g_hdc, color);

    TextOut(g_hdc, x, y, text.c_str(), strlen(text.c_str()));
}

bool initialize()
{
    std::cout << "WorkerW: ";
    if (!setWorkerW())
    {
        std::cout << "Error" << std::endl;
        return false;
    }
    std::cout << "0x" << std::hex << g_workerW << std::endl;

    std::cout << "Wallpaper: ";
    if (!getWallpaper())
    {
        std::cout << "Error" << std::endl;
        return false;
    }
    std::cout << g_path << std::endl;

    g_hdc = GetDC(g_workerW);

    std::cout << "Font: ";
    if (!setFont())
    {
        std::cout << "Error" << std::endl;
        return false;
    }
    std::cout << Font << std::endl;

    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)[](DWORD event) -> BOOL
    {
        switch (event)
        {
            // Ctrl + C イベントはexe実行のみ機能
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
        std::cout << "Ctrl Handle : Error" << std::endl;
        return false;
    }

    g_loop = true;

    return true;
}

bool finalize()
{
    ReleaseDC(g_workerW, g_hdc);

    std::cout << "Wallpaper: ";
    if (!setWallpaper(g_path))
    {
        std::cout << "Error" << std::endl;
        return false;
    }
    std::cout << "Reset" << std::endl;

    return true;
}

int main()
{
    if (!initialize()) return EXIT_FAILURE;

    /*for (int y = 0; y < MaxRow; y++)
    {
        for (int x = 0; x < MaxColumn; x++)
        {
            drawText(5 + (10 + 10) * x, 20 * y, Green, "q");
        }
    }*/

    int count = 0;
    while (g_loop)
    {
        clearScreen();
        drawText(5,  0 + count * 20, Green, "p");
        drawText(5, 20 + count * 20, Green, "q");
        drawText(5, 40 + count * 20, Green, "r");
        drawText(5, 60 + count * 20, Green, "s");
        drawText(5, 80 + count * 20, Green, "t");
        count++;
        Sleep(50);
    }

    if (!finalize()) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
