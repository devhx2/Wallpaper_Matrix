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

void drawText(int x, int y, const COLORREF color, const std::string text)
{
    // 文字の後ろは塗りつぶさない
    SetBkMode(g_hdc, TRANSPARENT);

    SetTextColor(g_hdc, color);

    TextOut(g_hdc, x, y, text.c_str(), strlen(text.c_str()));
}

int main()
{
    bool result = setWorkerW();
    std::cout << "WorkerW: ";
    if (!result)
    {
        std::cout << "Error" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "0x" << std::hex << g_workerW << std::endl;


    result = getWallpaper();
    std::cout << "Wallpaper: ";
    if (!result)
    {
        std::cout << "Error" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << g_path << std::endl;

    g_hdc = GetDC(g_workerW);

    result = setFont();
    std::cout << "Font: ";
    if (!result)
    {
        std::cout << "Error" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << Font << std::endl;

    //auto handler = [](DWORD event) -> BOOL
    //{
    //    if (event == CTRL_CLOSE_EVENT)
    //    {
    //        ReleaseDC(g_workerW, g_hdc);
    //        setWallpaper(g_path);
    //        return TRUE;
    //    }
    //    return FALSE;
    //};

    //// コンソールクローズイベントハンドラの設定
    //if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)handler, TRUE))
    //{
    //    std::cerr << "Error: Could not set control handler." << std::endl;
    //    return 1;
    //}

    clearScreen();
    for (int y = 0; y < MaxRow; y++)
    {
        for (int x = 0; x < MaxColumn; x++)
        {
            drawText(5 + (10 + 10) * x, 20 * y, Green, "q");
        }
    }

    std::cin.get();

    ReleaseDC(g_workerW, g_hdc);
    setWallpaper(g_path);

    return EXIT_SUCCESS;
}
