//#include "Screen.hpp"
//
//#include <cstdlib>
//#include <ctime>
//#include <iostream>
//
//bool Screen::m_loop;
//
//Screen::Screen()
//    : m_workerW(nullptr)
//    , m_buffer(nullptr)
//    , m_bitmap()
//{
//}
//
//Screen::~Screen()
//{
//    if (m_buffer)
//    {
//        DeleteDC(m_buffer);
//        m_buffer = nullptr;
//    }
//    if (m_bitmap)
//    {
//        DeleteObject(m_bitmap);
//        m_bitmap = nullptr;
//    }
//
//    std::cout << "Wallpaper: ";
//    if (!setWallpaper(m_path))
//    {
//        std::cout << "Error" << std::endl;
//        return;
//    }
//    std::cout << "Reset" << std::endl;
//}
//
//bool Screen::Initialize()
//{
//    std::cout << "WorkerW: ";
//    if (!setWorkerW())
//    {
//        std::cout << "Error" << std::endl;
//        return false;
//    }
//    std::cout << "0x" << std::hex << m_workerW << std::endl;
//    std::cout << std::dec;
//
//    std::cout << "Wallpaper: ";
//    if (!getWallpaper())
//    {
//        std::cout << "Error" << std::endl;
//        return false;
//    }
//    std::cout << m_path << std::endl;
//
//    std::cout << Font << std::endl;
//
//    std::cout << "Ctrl Handle: ";
//    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)[](DWORD event) -> BOOL
//    {
//        switch (event)
//        {
//            // イベントはexe実行のみ機能
//            // VisualStudioからの実行では機能しない
//            case CTRL_C_EVENT:
//            case CTRL_CLOSE_EVENT:
//            case CTRL_SHUTDOWN_EVENT:
//                m_loop = false;
//                return TRUE;
//            default:
//                return FALSE;
//        }
//    }, TRUE))
//    {
//        std::cout << "Error" << std::endl;
//        return EXIT_FAILURE;
//    }
//    std::cout << "Set" << std::endl;
//
//    m_loop = true;
//
//    srand((unsigned)time(NULL));
//
//    //const int columns = 96;
//    const int columns = (getWidth() - 5 * 2 /*両端*/ - (FontWidth * 2)) / FontWidth * 2;
//    for (int column = 0; column < columns; column++) m_lines.push_back(new Line(column));
//
//    // ダブルバッファの準備
//    {
//        HDC hdc = GetDC(m_workerW);
//        m_bitmap = CreateCompatibleBitmap(hdc, getWidth(), getHeight());
//        m_buffer = CreateCompatibleDC(nullptr);
//        SelectObject(m_buffer, m_bitmap);
//        ReleaseDC(m_workerW, hdc);
//    }
//
//    return true;
//}
//
//bool Screen::Update()
//{
//    for (const auto& line : m_lines) line->Update(this);
//
//    m_lines.insert(m_lines.end(), m_insert.begin(), m_insert.end());
//    m_insert.clear();
//
//    std::erase_if(m_lines, [this](Line* line)
//    {
//        if (std::find(m_delete.begin(), m_delete.end(), line) != m_delete.end())
//        {
//            delete line;
//            return true;
//        }
//        return false;
//    });
//
//    m_delete.clear();
//
//    return m_loop;
//}
//
//void Screen::Draw()
//{
//    // フォントセット
//    {
//        const HFONT font = CreateFont(FontHeight, FontWidth, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
//                                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
//                                      DRAFT_QUALITY, DEFAULT_PITCH, Font.c_str());
//        SelectObject(m_buffer, font);
//        DeleteObject(font);
//    }
//
//    for (const auto& line : m_lines) line->Draw(this);
//}
//
//void Screen::Clear()
//{
//    drawRectangle(0, 0, getWidth(), getHeight(), Color::Black);
//}
//
//void Screen::Flip()
//{
//    HDC hdc = GetDC(m_workerW);
//    BitBlt(hdc, 0, 0, getWidth(), getHeight(), m_buffer, 0, 0, SRCCOPY);
//    ReleaseDC(m_workerW, hdc);
//}
//
//bool Screen::setWorkerW()
//{
//    // デスクトップ画面を管理するウィンドウを取得
//    const HWND progman = GetShellWindow();
//
//    // メッセージを送ってWorkerWを生成させる
//    // 詳細：https://www.codeproject.com/Articles/856020/Draw-Behind-Desktop-Icons-in-Windows-plus
//    SendMessageTimeout(progman, 0x052C, NULL, NULL, SMTO_NORMAL, 1000, NULL);
//
//    // WorkerWは複数ある
//    // アイコン裏の描画はデスクトップ管理ウィンドウの次のWorkerW
//    m_workerW = GetNextWindow(progman, GW_HWNDPREV);
//
//    return m_workerW != NULL;
//}
//
//bool Screen::getWallpaper()
//{
//    char path[MAX_PATH]{};
//
//    // デスクトップの壁紙を取得
//    if (SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, (PVOID)path, NULL))
//    {
//        m_path = path;
//        return true;
//    }
//    return false;
//}
//
//bool Screen::setWallpaper(const std::string path)
//{
//    // デスクトップの壁紙を設定
//    return SystemParametersInfo(SPI_SETDESKWALLPAPER, NULL, (PVOID)path.c_str(),
//                                SPIF_UPDATEINIFILE | SPIF_SENDCHANGE) == TRUE;
//}
//
//int Screen::getWidth()
//{
//    RECT rect;
//    GetClientRect(m_workerW, &rect);
//    return rect.right - rect.left;
//}
//
//int Screen::getHeight()
//{
//    RECT rect;
//    GetClientRect(m_workerW, &rect);
//    return rect.bottom - rect.top;
//}
//
//void Screen::drawText(const int x, const int y, const Color color, const std::string text)
//{
//    // 文字の後ろは塗りつぶさない
//    SetBkMode(m_buffer, TRANSPARENT);
//
//    SetTextColor(m_buffer, (int)color);
//
//    TextOut(m_buffer, x, y, text.c_str(), strlen(text.c_str()));
//}
//
//void Screen::drawRectangle(const int x, const int y, const int w, const int h, const Color color)
//{
//    HBRUSH brush = CreateSolidBrush((int)color);
//    HBRUSH old = (HBRUSH)SelectObject(m_buffer, brush);
//
//    // brushの色で塗りつぶす
//    PatBlt(m_buffer, x, y, w, h, PATCOPY);
//
//    SelectObject(m_buffer, old);
//    DeleteObject(brush);
//}
//
//Screen::Line::Line(int column)
//{
//    m_column = column;
//    
//    // 画面外に上部に初期位置をセット
//    m_row = -8 - (rand() % 32);
//    m_data = std::string(4 + rand() % 32, ' ');
//}
//
//Screen::Line::~Line()
//{
//}
//
//void Screen::Line::Update(Screen* screen)
//{
//    m_row++;
//
//    const int length = m_data.length();
//    const int row = screen->getHeight() / FontHeight;
//
//    if ((m_row - length + 1) == row) screen->m_delete.push_back(this);
//
//    for (int index = length - 1; index >= 1; index--)
//    {
//        m_data[index] = m_data[index - 1];
//    }
//
//    // Asciiコードの制御文字と空白を除いてランダム決定
//    m_data[0] = (33 + rand() % 94);
//
//    // 画面に初めて入ったときのみ後続を生成
//    if ((m_row - length + 1) == 0) screen->m_insert.push_back(new Line(m_column));
//}
//
//void Screen::Line::Draw(Screen* screen)
//{
//    const int length = m_data.length();
//    for (int index = 0; index < length; index++)
//    {
//        int x = 5 /*端の調整*/ + m_column * FontWidth * 2;
//        int y = (m_row - index) * FontHeight;
//        Color color = index == 0 ? Color::Gray : Color::Green;
//        screen->drawText(x, y, color, std::string(1, m_data[index]));
//    }
//}
