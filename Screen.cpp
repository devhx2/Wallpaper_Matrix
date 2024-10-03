#include "Screen.hpp"

#include <cstdlib>
#include <ctime>
#include <iostream>

bool Screen::m_loop;

Screen::Screen()
    : m_workerW(nullptr)
    , m_buffer(nullptr)
    , m_bitmap()
{
}

Screen::~Screen()
{
    if (m_buffer)
    {
        DeleteDC(m_buffer);
        m_buffer = nullptr;
    }
    if (m_bitmap)
    {
        DeleteObject(m_bitmap);
        m_bitmap = nullptr;
    }

    std::cout << "Wallpaper: ";
    if (!setWallpaper(m_path))
    {
        std::cout << "Error" << std::endl;
        return;
    }
    std::cout << "Reset" << std::endl;
}

bool Screen::Initialize()
{
    std::cout << "WorkerW: ";
    if (!setWorkerW())
    {
        std::cout << "Error" << std::endl;
        return false;
    }
    std::cout << "0x" << std::hex << m_workerW << std::endl;
    std::cout << std::dec;

    std::cout << "Wallpaper: ";
    if (!getWallpaper())
    {
        std::cout << "Error" << std::endl;
        return false;
    }
    std::cout << m_path << std::endl;

    std::cout << "Ctrl Handle: ";
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)[](DWORD event) -> BOOL
    {
        switch (event)
        {
            // イベントはexe実行のみ機能
            // VisualStudioからの実行では機能しない
            case CTRL_C_EVENT:
            case CTRL_CLOSE_EVENT:
            case CTRL_SHUTDOWN_EVENT:
                m_loop = false;
                return TRUE;
            default:
                return FALSE;
        }
    }, TRUE))
    {
        std::cout << "Error" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Set" << std::endl;

    m_loop = true;

    srand((unsigned)time(NULL));

    for (int column = 0; column < MaxColumn; column++) m_lines.push_back(Line(column * 2));

    for (int row = 0; row < MaxRow; row++) m_strings.push_back(std::string(MaxColumn * 2, ' '));

    // ダブルバッファの準備
    {
        HDC hdc = GetDC(m_workerW);
        m_bitmap = CreateCompatibleBitmap(hdc, ScreenWidth, ScreenHeight);
        m_buffer = CreateCompatibleDC(nullptr);
        SelectObject(m_buffer, m_bitmap);
        ReleaseDC(m_workerW, hdc);
    }

    return true;
}

bool Screen::Update()
{
    for (auto iterator = m_lines.begin(); iterator != m_lines.end();)
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

        if (TopRow == 0) m_lines.push_back(Line(Column));

        if (MaxRow - 1 < TopRow)
        {
            iterator = m_lines.erase(iterator);
            continue;
        }

        if (BottomRow < MaxRow) m_strings[BottomRow][Column] = 33 + rand() % 94;

        if (0 <= TopRow - 1) m_strings[TopRow - 1][Column] = ' ';

        iterator++;
    }

    Sleep(WaitTime);

    return m_loop;
}

void Screen::Draw()
{
    // フォントセット
    {
        const HFONT font = CreateFont(FontHeight, FontWidth, 0, 0,
                                      FW_DONTCARE, FALSE, FALSE, FALSE,
                                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      DRAFT_QUALITY, DEFAULT_PITCH, "Cascadia Mono SemiBold");
        SelectObject(m_buffer, font);
        DeleteObject(font);
    }

    SetBkMode(m_buffer, TRANSPARENT);

    SetTextColor(m_buffer, (int)Color::Green);

    for (int row = 0; row < MaxRow; row++)
    {
        TextOut(m_buffer, 5, row * FontHeight, m_strings[row].c_str(), m_strings[row].length());
    }
}

void Screen::Clear()
{
    const HBRUSH brush = CreateSolidBrush((int)Color::Black);
    const HBRUSH old = (HBRUSH)SelectObject(m_buffer, brush);

    // brushの色で塗りつぶす
    PatBlt(m_buffer, 0, 0, ScreenWidth, ScreenHeight, PATCOPY);

    SelectObject(m_buffer, old);
    DeleteObject(brush);
}

void Screen::Flip()
{
    const HDC hdc = GetDC(m_workerW);
    BitBlt(hdc, 0, 0, ScreenWidth, ScreenHeight, m_buffer, 0, 0, SRCCOPY);
    ReleaseDC(m_workerW, hdc);
}

bool Screen::setWorkerW()
{
    // デスクトップ画面を管理するウィンドウを取得
    const HWND progman = GetShellWindow();

    // メッセージを送ってWorkerWを生成させる
    // 詳細：https://www.codeproject.com/Articles/856020/Draw-Behind-Desktop-Icons-in-Windows-plus
    SendMessageTimeout(progman, 0x052C, NULL, NULL, SMTO_NORMAL, 1000, NULL);

    // WorkerWは複数ある
    // アイコン裏の描画はデスクトップ管理ウィンドウの次のWorkerW
    m_workerW = GetNextWindow(progman, GW_HWNDPREV);

    return m_workerW != NULL;
}

bool Screen::getWallpaper()
{
    char path[MAX_PATH]{};

    // デスクトップの壁紙を取得
    if (SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, (PVOID)path, NULL))
    {
        m_path = path;
        return true;
    }
    return false;
}

bool Screen::setWallpaper(const std::string path)
{
    // デスクトップの壁紙を設定
    return SystemParametersInfo(SPI_SETDESKWALLPAPER, NULL, (PVOID)path.c_str(),
                                SPIF_UPDATEINIFILE | SPIF_SENDCHANGE) == TRUE;
}
