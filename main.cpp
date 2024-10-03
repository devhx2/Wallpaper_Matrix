#include <windows.h>

#include <cstdlib>
#include <ctime>
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

const int ScreenWidth = 1920;
const int ScreenHeight = 1080;
const int FontHeight = 20;
const int FontWidth = FontHeight / 2;
const int MaxColumn = ScreenWidth / (FontWidth * 2);
const int MaxRow = ScreenHeight / FontHeight;
const int WaitTime = 1000 /*ms*/ / 10 /*fps*/;

HWND g_workerW;
HDC g_buffer;
HBITMAP g_bitmap;
std::string g_path;
std::list<Line> g_lines;
std::vector<std::string> g_strings;
std::vector<std::string> g_strings2;

bool Initialize();
void Update();
void Draw();
bool Finalize();

int main()
{
    if (!Initialize()) return EXIT_FAILURE;

    while (true)
    {
        Update();
        Draw();

        Sleep(WaitTime);
    }

    if (!Finalize()) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

bool Initialize()
{
    // 壁紙の描画用ウィンドウハンドルを取得
    {
        // デスクトップ画面を管理するウィンドウを取得
        const HWND progman = GetShellWindow();

        // メッセージを送ってWorkerWを生成させる
        SendMessageTimeout(progman, 0x052C, NULL, NULL, SMTO_NORMAL, 1000, NULL);

        // WorkerWは複数あるが壁紙の描画用はデスクトップ管理ウィンドウの次のWorkerW
        g_workerW = GetNextWindow(progman, GW_HWNDPREV);

        if (g_workerW == NULL)
        {
            std::cerr << "Error : WorkerW" << std::endl;
            return false;
        }
    }
    // デスクトップの壁紙を取得
    {
        char path[MAX_PATH]{};
        if (!SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, (PVOID)path, NULL))
        {
            std::cerr << "Error : Get Wallpaper Path" << std::endl;
            return false;
        }
        g_path = path;
    }
    // ダブルバッファの準備
    {
        const HDC hdc = GetDC(g_workerW);
        g_bitmap = CreateCompatibleBitmap(hdc, ScreenWidth, ScreenHeight);
        g_buffer = CreateCompatibleDC(nullptr);
        SelectObject(g_buffer, g_bitmap);
        ReleaseDC(g_workerW, hdc);
    }
    // イベントを登録
    {
        if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)[](DWORD event) -> BOOL
        {
            switch (event)
            {
                // イベントはexe実行のみ機能
                // VisualStudioからの実行では機能しない
                case CTRL_C_EVENT:
                case CTRL_CLOSE_EVENT:
                case CTRL_SHUTDOWN_EVENT:
                    Finalize();
                    return TRUE;
                default:
                    return FALSE;
            }
        }, TRUE))
        {
            std::cerr << "Error : Ctrl Handle" << std::endl;
            return false;
        }
    }
    // 文字列バッファの準備
    {
        srand((unsigned)time(NULL));

        for (int column = 0; column < MaxColumn; column++) g_lines.push_back(Line(column * 2));

        for (int row = 0; row < MaxRow; row++)
        {
            g_strings.push_back(std::string(MaxColumn * 2, ' '));
            g_strings2.push_back(std::string(MaxColumn * 2, ' '));
        }
    }
    // コンソール準備
    {
        const HWND hwnd = GetConsoleWindow();
        SetWindowText(hwnd, "Wallpaper_Matrix");
    }

    return true;
}

void Update()
{
    for (int row = 0; row < MaxRow; row++)
    {
        for (int column = 0; column < MaxColumn * 2; column++)
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

        if (MaxRow - 1 < TopRow)
        {
            iterator = g_lines.erase(iterator);
            continue;
        }

        if (BottomRow < MaxRow) g_strings2[BottomRow][Column] = 33 + rand() % 94;

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

        PatBlt(g_buffer, 0, 0, ScreenWidth, ScreenHeight, PATCOPY);

        SelectObject(g_buffer, old);
        DeleteObject(brush);
    }
    // フォントセット
    {
        const HFONT font = CreateFont(FontHeight, FontWidth, 0, 0,
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
        for (int row = 0; row < MaxRow; row++)
        {
            TextOut(g_buffer, 5, row * FontHeight, g_strings[row].c_str(), g_strings[row].length());
        }

        SetTextColor(g_buffer, RGB(192, 192, 192));
        for (int row = 0; row < MaxRow; row++)
        {
            TextOut(g_buffer, 5, row * FontHeight, g_strings2[row].c_str(), g_strings2[row].length());
        }
    }
    // ダブルバッファを入れ替え
    {
        const HDC hdc = GetDC(g_workerW);
        BitBlt(hdc, 0, 0, ScreenWidth, ScreenHeight, g_buffer, 0, 0, SRCCOPY);
        ReleaseDC(g_workerW, hdc);
    }
}

bool Finalize()
{
    // 変数破棄
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
    }
    // デスクトップの壁紙を戻す
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
