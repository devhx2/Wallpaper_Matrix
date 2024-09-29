#include "Screen.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>

bool Screen::m_loop;

Screen::Screen()
    : m_workerW(nullptr)
    , m_hdc(nullptr)
{
}

Screen::~Screen()
{
    ReleaseDC(m_workerW, m_hdc);

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

    std::cout << "Wallpaper: ";
    if (!getWallpaper())
    {
        std::cout << "Error" << std::endl;
        return false;
    }
    std::cout << m_path << std::endl;

    m_hdc = GetDC(m_workerW);

    std::cout << "Font: ";
    if (!setFont())
    {
        std::cout << "Error" << std::endl;
        return false;
    }
    std::cout << Font << std::endl;

    std::cout << "Ctrl Handle: ";
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)[](DWORD event) -> BOOL
    {
        switch (event)
        {
            // �C�x���g��exe���s�̂݋@�\
            // VisualStudio����̎��s�ł͋@�\���Ȃ�
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

    m_lines.push_back(Line(0));

    return true;
}

bool Screen::Update()
{
    std::for_each(m_lines.begin(), m_lines.end(), [](Line &line) { line.Update(); });

    Sleep(WaitTime);

    return m_loop;
}

void Screen::Draw()
{
    std::for_each(m_lines.begin(), m_lines.end(), [this](Line &line) { line.Draw(this); });
}

void Screen::Clear()
{
    drawRectangle(0, 0, ScreenWidth, ScreenHeight, Color::Black);
}

bool Screen::setWorkerW()
{
    // �f�X�N�g�b�v��ʂ��Ǘ�����E�B���h�E���擾
    const HWND progman = GetShellWindow();

    // ���b�Z�[�W�𑗂���WorkerW�𐶐�������
    // �ڍׁFhttps://www.codeproject.com/Articles/856020/Draw-Behind-Desktop-Icons-in-Windows-plus
    SendMessageTimeout(progman, 0x052C, NULL, NULL, SMTO_NORMAL, 1000, NULL);

    // WorkerW�͕�������
    // �A�C�R�����̕`��̓f�X�N�g�b�v�Ǘ��E�B���h�E�̎���WorkerW
    m_workerW = GetNextWindow(progman, GW_HWNDPREV);

    return m_workerW != NULL;
}

bool Screen::getWallpaper()
{
    char path[MAX_PATH]{};

    // �f�X�N�g�b�v�̕ǎ����擾
    if (SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, (PVOID)path, NULL))
    {
        m_path = path;
        return true;
    }
    return false;
}

bool Screen::setWallpaper(const std::string path)
{
    // �f�X�N�g�b�v�̕ǎ���ݒ�
    return SystemParametersInfo(SPI_SETDESKWALLPAPER, NULL, (PVOID)path.c_str(),
                                SPIF_UPDATEINIFILE | SPIF_SENDCHANGE) == TRUE;
}

bool Screen::setFont()
{
    const HFONT font = CreateFont(FontHeight, FontWidth, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
                                  ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  DRAFT_QUALITY, DEFAULT_PITCH, Font.c_str());
    SelectObject(m_hdc, font);
    DeleteObject(font);

    return true;
}

void Screen::drawText(const int x, const int y, const Color color, const std::string text)
{
    // �����̌��͓h��Ԃ��Ȃ�
    SetBkMode(m_hdc, TRANSPARENT);

    SetTextColor(m_hdc, (int)color);

    TextOut(m_hdc, x, y, text.c_str(), strlen(text.c_str()));
}

void Screen::drawRectangle(const int x, const int y, const int w, const int h, const Color color)
{
    const HBRUSH brush = CreateSolidBrush((int)color);
    const HBRUSH old = (HBRUSH)SelectObject(m_hdc, brush);

    // brush�̐F�œh��Ԃ�
    PatBlt(m_hdc, x, y, w, h, PATCOPY);

    SelectObject(m_hdc, old);
    DeleteObject(brush);
}

Screen::Line::Line(int column)
{
    m_column = column;
    
    // ��ʊO�ɏ㕔�ɏ����ʒu���Z�b�g
    m_row = -8 - (rand() % MaxRow) / 2;

    m_data = std::string(8 + rand() % (MaxRow - 8), '\0');
}

Screen::Line::~Line()
{
}

void Screen::Line::Update()
{
    m_row++;

    for (int index = m_data.size() - 1; index >= 1; index--)
    {
        m_data[index] = m_data[index - 1];
    }

    // Ascii�R�[�h�̐��䕶���Ƌ󔒂������ă����_������
    m_data[0] = (33 + rand() % 94);
}

void Screen::Line::Draw(Screen* screen)
{
    for (int index = 0; index < m_data.size(); index++)
    {
        int x = 5 /*�[�̒���*/ + m_column * FontWidth;
        int y = (m_row - index) * FontHeight;
        Color color = index == 0 ? Color::Gray : Color::Green;
        screen->drawText(x, y, color, std::string(1, m_data[index]));
    }
}
