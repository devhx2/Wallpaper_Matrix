#include <windows.h>

#include <functional>
#include <iostream>
#include <string>

namespace utility
{
    HWND getWorkerW()
    {
        // デスクトップ画面を管理するウィンドウを取得
        const HWND progman = GetShellWindow();
        std::cout << "Progman: " << progman << std::endl;

        // メッセージを送ってWorkerWを生成させる
        // 詳細：https://www.codeproject.com/Articles/856020/Draw-Behind-Desktop-Icons-in-Windows-plus
        SendMessageTimeout(progman, 0x052C, NULL, NULL, SMTO_NORMAL, 1000, NULL);

        // WorkerWは複数ある
        // アイコン裏の描画はデスクトップ管理ウィンドウの次のWorkerW
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

        // デスクトップの壁紙を取得
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

        // デスクトップの壁紙を設定
        if (SystemParametersInfo(SPI_SETDESKWALLPAPER, NULL, (PVOID)path.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE)) std::cout << "OK" << std::endl;
        else std::cout << "Error" << std::endl;
    }
}

