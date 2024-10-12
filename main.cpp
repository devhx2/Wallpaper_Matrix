#include <windows.h>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <list>
#include <map>
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

Size g_screenSize;
Size g_fontSize;
int g_columnNum;
int g_rowNum;
HWND g_workerW;
HDC g_buffer;
HBITMAP g_bitmap;
std::string g_path;
std::list<Line> g_lines;
std::vector<std::string> g_strings;
std::vector<std::string> g_strings2;

enum Mode
{
    Alphabet,
    Binary,
};

Mode g_mode = Mode::Alphabet;
int g_speed = 7;
COLORREF g_fgColor = RGB(19, 161, 14);
COLORREF g_bgColor = RGB(0, 0, 0);

bool Initialize();
void CalcParam();
void Update();
void Draw();
bool Finalize();

int main(int argc, char* argv[])
{
    if (!Initialize()) return EXIT_FAILURE;

    // オプションを設定
    for (int i = 0; i < argc; i++)
    {
        std::string str = argv[i];

        // 文字を0,1に限定
        if (str == "/bin") g_mode = Mode::Binary;

        // 落下速度を1~9で指定
        if (str == "/spd" && i + 1 < argc)
        {
            int spd = std::stoi(argv[i + 1]);
            g_speed = std::clamp(spd, 1, 9);
        }

        // 前景色を16進数で指定
        if (str == "/fgc" && i + 1 < argc)
        {
            int fgc = std::stoi(argv[i + 1], nullptr, 16);
            int r = (fgc >> 16) & 0xff;
            int g = (fgc >> 8) & 0xff;
            int b = fgc & 0xff;
            g_fgColor = RGB(r, g, b);
        }

        // 後景色を16進数で指定
        if (str == "/bgc" && i + 1 < argc)
        {
            int bgc = std::stoi(argv[i + 1], nullptr, 16);
            int r = (bgc >> 16) & 0xff;
            int g = (bgc >> 8) & 0xff;
            int b = bgc & 0xff;
            g_bgColor = RGB(r, g, b);
        }
    }

    CalcParam();

    int count = 0;

    while (true)
    {
        count = (count + 1) % (10/*max*/ - g_speed);
        if (count == 0)
        {
            CalcParam();
            Update();
            Draw();
        }

        Sleep(10/*ms*/);
    }

    return EXIT_SUCCESS;
}

bool Initialize()
{
    // 壁紙の描画用ウィンドウハンドルを取得
    {
        // デスクトップ画面を管理するウィンドウを取得
        const HWND Hwnd = GetShellWindow();

        // メッセージを送ってWorkerWを生成させる
        SendMessageTimeoutA(Hwnd, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);

        // WorkerWは複数あるが壁紙の描画用はデスクトップ管理ウィンドウの次のWorkerW
        g_workerW = GetWindow(Hwnd, GW_HWNDPREV);

        if (g_workerW == nullptr)
        {
            std::cerr << "Error : WorkerW" << std::endl;
            return false;
        }
    }
    // デスクトップの壁紙を取得
    {
        char path[MAX_PATH]{};

        if (!SystemParametersInfoA(SPI_GETDESKWALLPAPER, MAX_PATH, (PVOID)path, 0))
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
    return true;
}

void CalcParam()
{
    // パラメータの算出
    {
        static RECT oldScreenSize{ 0, 0, 0, 0 }, nowScreenSize;
        GetClientRect(g_workerW, &nowScreenSize);

        const int OldWidth = oldScreenSize.right - oldScreenSize.left;
        const int OldHeight = oldScreenSize.bottom - oldScreenSize.top;
        const int NowWidth = nowScreenSize.right - nowScreenSize.left;
        const int NowHeight = nowScreenSize.bottom - nowScreenSize.top;
        
        if (OldWidth == NowWidth && OldHeight == NowHeight) return;

        g_fontSize = { 10, 20 };
        g_screenSize.Width = NowWidth;
        g_screenSize.Height = NowHeight;
        g_columnNum = g_screenSize.Width / (g_fontSize.Width * 2);
        g_rowNum = g_screenSize.Height / g_fontSize.Height;

        oldScreenSize = nowScreenSize;
    }
    // ダブルバッファの準備
    {
        const HDC Hdc = GetDC(g_workerW);

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

        g_bitmap = CreateCompatibleBitmap(Hdc, g_screenSize.Width, g_screenSize.Height);
        g_buffer = CreateCompatibleDC(0);

        SelectObject(g_buffer, g_bitmap);
        ReleaseDC(g_workerW, Hdc);
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

        if (BottomRow < g_rowNum)
        {
            if (g_mode == Mode::Alphabet) g_strings2[BottomRow][Column] = 33 + rand() % 94;
            else g_strings2[BottomRow][Column] = 48 + rand() % 2;
        }

        if (0 <= TopRow - 1) g_strings[TopRow - 1][Column] = ' ';

        iterator++;
    }
}

void Draw()
{
    // 背景の塗りつぶし
    {
        const HBRUSH Brush = CreateSolidBrush(g_bgColor);
        const HBRUSH OldBrush = (HBRUSH)SelectObject(g_buffer, Brush);

        PatBlt(g_buffer, 0, 0, g_screenSize.Width, g_screenSize.Height, PATCOPY);

        SelectObject(g_buffer, OldBrush);
        DeleteObject(Brush);
    }
    // 文字列の描画
    {
        const HFONT Font = CreateFontA(g_fontSize.Height, g_fontSize.Width,
                                      0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
                                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      DRAFT_QUALITY, DEFAULT_PITCH, "Cascadia Mono SemiBold");
        const HFONT OldFont = (HFONT)SelectObject(g_buffer, Font);

        SetBkMode(g_buffer, TRANSPARENT);

        SetTextColor(g_buffer, g_fgColor);
        for (int row = 0; row < g_rowNum; row++)
        {
            TextOutA(g_buffer, 5, row * g_fontSize.Height, g_strings[row].c_str(), g_strings[row].length());
        }

        SetTextColor(g_buffer, RGB(192, 192, 192));
        for (int row = 0; row < g_rowNum; row++)
        {
            TextOutA(g_buffer, 5, row * g_fontSize.Height, g_strings2[row].c_str(), g_strings2[row].length());
        }

        SelectObject(g_buffer, OldFont);
        DeleteObject(Font);
    }
    // ダブルバッファを入れ替え
    {
        const HDC Hdc = GetDC(g_workerW);
        BitBlt(Hdc, 0, 0, g_screenSize.Width, g_screenSize.Height, g_buffer, 0, 0, SRCCOPY);
        ReleaseDC(g_workerW, Hdc);
    }
}

bool Finalize()
{
    if (g_buffer != nullptr) DeleteDC(g_buffer);

    if (g_bitmap != nullptr) DeleteObject(g_bitmap);

    // デスクトップの壁紙を設定
    {
        if (!SystemParametersInfoA(SPI_SETDESKWALLPAPER, NULL,
            (PVOID)g_path.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE))
        {
            std::cerr << "Error : Set Wallpaper Path" << std::endl;
            return false;
        }
    }

    return true;
}
