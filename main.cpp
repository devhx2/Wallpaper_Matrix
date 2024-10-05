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

Size g_screenSize/* = { 1920, 1080 }*/;
Size g_FontSize = { 10, 20 };
int g_columnNum/* = g_screenSize.Width / (g_FontSize.Width * 2)*/;
int g_rowNum/* = g_screenSize.Height / g_FontSize.Height*/;
int g_waitTime = 1000 /*ms*/ / 50 /*fps*/;

HWND g_workerW;
HDC g_buffer;
HBITMAP g_bitmap;
HANDLE g_console;
std::string g_path;
std::list<Line> g_lines;
std::vector<std::string> g_strings;
std::vector<std::string> g_strings2;

bool Initialize();
void CalcParam();
void Update();
void Draw();
bool Finalize();

int main()
{
    if (!Initialize()) return EXIT_FAILURE;

    CalcParam();

    std::future<void> wallpaper = std::async([]()
    {
        while (true)
        {
            Update();
            Draw();

            Sleep(g_waitTime);

            CalcParam();
        }
    });

    wallpaper.get();

    return EXIT_SUCCESS;
}

bool Initialize()
{
    // 壁紙の描画用ウィンドウハンドルを取得
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
    }
    // デスクトップの壁紙を取得
    {
        char path[MAX_PATH]{};

        if (!SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, (PVOID)path, 0))
        {
            std::cerr << "Error : Get Wallpaper Path" << std::endl;
            return false;
        }

        g_path = path;
    }
    // イベントを登録
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
    }
    // コンソール準備
    {
        const HWND hwnd = GetConsoleWindow();
        SetWindowText(hwnd, "Wallpaper_Matrix");
    }
    return true;
}

void CalcParam()
{
    // パラメータの算出
    {
        static RECT old{ 0, 0, 0, 0 }, now;
        GetClientRect(g_workerW, &now);

        int ow = old.right - old.left;
        int oh = old.bottom - old.top;
        int nw = now.right - now.left;
        int nh = now.bottom - now.top;
        
        if (ow == nw && oh == nh) return;

        g_screenSize.Width = nw;
        g_screenSize.Height = nh;
        g_columnNum = g_screenSize.Width / (g_FontSize.Width * 2);
        g_rowNum = g_screenSize.Height / g_FontSize.Height;

        std::cout << "g_screenSize.Width  " << g_screenSize.Width << std::endl;
        std::cout << "g_screenSize.Height " << g_screenSize.Height << std::endl;
        std::cout << "g_columnNum         " << g_columnNum << std::endl;
        std::cout << "g_rowNum            " << g_rowNum << std::endl;

        old = now;
    }
    // ダブルバッファの準備
    {
        const HDC hdc = GetDC(g_workerW);

        if (g_buffer != nullptr)
        {
            DeleteDC(g_buffer);
            g_buffer = nullptr;
        }

        if (g_bitmap != nullptr)
        {
            DeleteObject(g_bitmap);
            g_bitmap = nullptr;
        }

        g_bitmap = CreateCompatibleBitmap(hdc, g_screenSize.Width, g_screenSize.Height);
        g_buffer = CreateCompatibleDC(0);

        SelectObject(g_buffer, g_bitmap);
        ReleaseDC(g_workerW, hdc);
    }
    // 文字列バッファの準備
    {
        srand((unsigned)time(nullptr));

        g_lines.clear();
        for (int column = 0; column < g_columnNum; column++) g_lines.push_back(Line(column * 2));

        g_strings.clear();
        g_strings2.clear();
        for (int row = 0; row < g_rowNum; row++)
        {
            g_strings.push_back(std::string(g_columnNum * 2, ' '));
            g_strings2.push_back(std::string(g_columnNum * 2, ' '));
        }
    }
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
    // 文字列の描画
    {
        const HFONT font = CreateFont(g_FontSize.Height, g_FontSize.Width,
                                      0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
                                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      DRAFT_QUALITY, DEFAULT_PITCH, "Cascadia Mono SemiBold");
        const HFONT old = (HFONT)SelectObject(g_buffer, font);

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

        SelectObject(g_buffer, old);
        DeleteObject(font);
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
    if (g_buffer != nullptr) DeleteDC(g_buffer);

    if (g_bitmap != nullptr) DeleteObject(g_bitmap);

    // デスクトップの壁紙を設定
    {
        if (!SystemParametersInfo(SPI_SETDESKWALLPAPER, NULL,
            (PVOID)g_path.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE))
        {
            std::cerr << "Error : Set Wallpaper Path" << std::endl;
            return false;
        }
    }

    return true;
}
