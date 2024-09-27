#include <windows.h>

#include <functional>
#include <iostream>
#include <string>

namespace utility
{
    HWND getWorkerW()
    {
        // �f�X�N�g�b�v��ʂ��Ǘ�����E�B���h�E���擾
        const HWND progman = GetShellWindow();
        std::cout << "Progman: " << progman << std::endl;

        // ���b�Z�[�W�𑗂���WorkerW�𐶐�������
        // �ڍׁFhttps://www.codeproject.com/Articles/856020/Draw-Behind-Desktop-Icons-in-Windows-plus
        SendMessageTimeout(progman, 0x052C, NULL, NULL, SMTO_NORMAL, 1000, NULL);

        // WorkerW�͕�������
        // �A�C�R�����̕`��̓f�X�N�g�b�v�Ǘ��E�B���h�E�̎���WorkerW
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

        // �f�X�N�g�b�v�̕ǎ����擾
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

        // �f�X�N�g�b�v�̕ǎ���ݒ�
        if (SystemParametersInfo(SPI_SETDESKWALLPAPER, NULL, (PVOID)path.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE)) std::cout << "OK" << std::endl;
        else std::cout << "Error" << std::endl;
    }
}

