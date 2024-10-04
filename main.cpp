#include <comdef.h>
#include <wbemidl.h>
#include <windows.h>

#include <cstdlib>
#include <ctime>
#include <future>
#include <iostream>
#include <list>
#include <string>
#include <vector>

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

struct Size
{
    int Width;
    int Height;
};

//const int ScreenWidth = 1920;
//const int ScreenHeight = 1080;
//const int FontHeight = 20;
//const int FontWidth = FontHeight / 2;
//const int MaxColumn = ScreenWidth / (FontWidth * 2);
//const int MaxRow = ScreenHeight / FontHeight;
//const int WaitTime = 1000 /*ms*/ / 20 /*fps*/;

Size g_screenSize = { 1920, 1080 };
Size g_FontSize = { 10, 20 };
int g_columnNum = g_screenSize.Width / (g_FontSize.Width * 2);
int g_rowNum = g_screenSize.Height / g_FontSize.Height;
int g_waitTime = 1000 /*ms*/ / 20 /*fps*/;

HWND g_workerW;
HDC g_buffer;
HBITMAP g_bitmap;
HANDLE g_console;
std::string g_path;
std::list<Line> g_lines;
std::vector<std::string> g_strings;
std::vector<std::string> g_strings2;

bool Initialize();
void Update();
void Draw();
bool Finalize();

bool InitWorkerW();
bool GetWallpaper();
bool SetWallpaper();
bool InitDoubleBuffer();
bool RegisterEvent();
bool InitStrings();
bool InitConsole();

int main()
{
    if (!Initialize()) return EXIT_FAILURE;

    std::future<void> wallpaper = std::async([]()
    {
        while (true)
        {
            Update();
            Draw();

            Sleep(g_waitTime);
        }
    });

    std::future<void> ui = std::async([]()
    {
        while (true)
        {
            std::cout << "x\n";

            Sleep(1000);
        }
    });

    wallpaper.get();
    ui.get();

    return EXIT_SUCCESS;
}

bool Initialize()
{
    if (!InitWorkerW())      return false;

    if (!GetWallpaper())     return false;

    if (!InitDoubleBuffer()) return false;

    if (!RegisterEvent())    return false;

    if (!InitStrings())      return false;

    return true;
}

void Update()
{
    for (int row = 0; row < g_rowNum; row++)
    {
        for (int column = 0; column < g_columnNum * 2; column++)
        {
            if (g_strings2[row][column] == ' ') continue;

            g_strings[row][column] = g_strings2[row][column];
            g_strings2[row][column] = ' ';
        }
    }

    for (auto iterator = g_lines.begin(); iterator != g_lines.end();)
    {
        const int Length = iterator->Length;
        const int TopRow = ++iterator->Row;
        const int BottomRow = TopRow + Length - 1;
        const int Column = iterator->Column;

        if (BottomRow < 0)
        {
            iterator++;
            continue;
        }

        if (TopRow == 0) g_lines.push_back(Line(Column));

        if (g_rowNum - 1 < TopRow)
        {
            iterator = g_lines.erase(iterator);
            continue;
        }

        if (BottomRow < g_rowNum) g_strings2[BottomRow][Column] = 33 + rand() % 94;

        if (0 <= TopRow - 1) g_strings[TopRow - 1][Column] = ' ';

        iterator++;
    }
}

void Draw()
{
    // 背景の塗りつぶし
    {
        const HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
        const HBRUSH old = (HBRUSH)SelectObject(g_buffer, brush);

        PatBlt(g_buffer, 0, 0, g_screenSize.Width, g_screenSize.Height, PATCOPY);

        SelectObject(g_buffer, old);
        DeleteObject(brush);
    }
    // フォントセット
    {
        const HFONT font = CreateFont(g_FontSize.Height, g_FontSize.Width, 0, 0,
                                      FW_DONTCARE, FALSE, FALSE, FALSE,
                                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      DRAFT_QUALITY, DEFAULT_PITCH, "Cascadia Mono SemiBold");
        SelectObject(g_buffer, font);
        DeleteObject(font);
    }
    // 文字列の描画
    {
        SetBkMode(g_buffer, TRANSPARENT);

        SetTextColor(g_buffer, RGB(19, 161, 14));
        for (int row = 0; row < g_rowNum; row++)
        {
            TextOut(g_buffer, 5, row * g_FontSize.Height, g_strings[row].c_str(), g_strings[row].length());
        }

        SetTextColor(g_buffer, RGB(192, 192, 192));
        for (int row = 0; row < g_rowNum; row++)
        {
            TextOut(g_buffer, 5, row * g_FontSize.Height, g_strings2[row].c_str(), g_strings2[row].length());
        }
    }
    // ダブルバッファを入れ替え
    {
        const HDC hdc = GetDC(g_workerW);
        BitBlt(hdc, 0, 0, g_screenSize.Width, g_screenSize.Height, g_buffer, 0, 0, SRCCOPY);
        ReleaseDC(g_workerW, hdc);
    }
}

bool Finalize()
{
    if (g_buffer)
    {
        DeleteDC(g_buffer);
        g_buffer = nullptr;
    }
    if (g_bitmap)
    {
        DeleteObject(g_bitmap);
        g_bitmap = nullptr;
    }

    SetWallpaper();

    return true;
}

// 壁紙の描画用ウィンドウハンドルを取得
bool InitWorkerW()
{
    // デスクトップ画面を管理するウィンドウを取得
    const HWND hwnd = GetShellWindow();

    // メッセージを送ってWorkerWを生成させる
    SendMessageTimeout(hwnd, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);

    // WorkerWは複数あるが壁紙の描画用はデスクトップ管理ウィンドウの次のWorkerW
    g_workerW = GetNextWindow(hwnd, GW_HWNDPREV);

    if (g_workerW == nullptr)
    {
        std::cerr << "Error : WorkerW" << std::endl;
        return false;
    }

    return true;
}

// デスクトップの壁紙を取得
bool GetWallpaper()
{
    char path[MAX_PATH]{};

    if (!SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, (PVOID)path, 0))
    {
        std::cerr << "Error : Get Wallpaper Path" << std::endl;
        return false;
    }

    g_path = path;

    return true;
}

// デスクトップの壁紙を設定
bool SetWallpaper()
{
    if (!SystemParametersInfo(SPI_SETDESKWALLPAPER, NULL,
        (PVOID)g_path.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE))
    {
        std::cerr << "Error : Set Wallpaper Path" << std::endl;
        return false;
    }

    return true;
}

// ダブルバッファの準備
bool InitDoubleBuffer()
{
    const HDC hdc = GetDC(g_workerW);

    g_bitmap = CreateCompatibleBitmap(hdc, g_screenSize.Width, g_screenSize.Height);
    g_buffer = CreateCompatibleDC(0);

    SelectObject(g_buffer, g_bitmap);
    ReleaseDC(g_workerW, hdc);

    return true;
}

// イベントを登録
bool RegisterEvent()
{
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)[](DWORD event) -> BOOL
    {
        switch (event)
        {
            case CTRL_C_EVENT:
            case CTRL_CLOSE_EVENT:
            case CTRL_SHUTDOWN_EVENT:
                Finalize();
                return FALSE;
            default:
                return FALSE;
        }
    }, TRUE))
    {
        std::cerr << "Error : Ctrl Handle" << std::endl;
        return false;
    }

    return true;
}

// 文字列バッファの準備
bool InitStrings()
{
    srand((unsigned)time(nullptr));

    for (int column = 0; column < g_columnNum; column++) g_lines.push_back(Line(column * 2));

    for (int row = 0; row < g_rowNum; row++)
    {
        g_strings.push_back(std::string(g_columnNum * 2, ' '));
        g_strings2.push_back(std::string(g_columnNum * 2, ' '));
    }

    return true;
}

// コンソール準備
bool InitConsole()
{
    const HWND hwnd = GetConsoleWindow();
    SetWindowText(hwnd, "Wallpaper_Matrix");

    // 標準出力ハンドルを取得
    g_console = GetStdHandle(STD_OUTPUT_HANDLE);

    // カーソル位置をセット
    SetConsoleCursorPosition(g_console, { 20,20 });

    return true;
}
