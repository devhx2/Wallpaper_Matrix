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
        std::cout << "Progman:" << progman << std::endl;

        // メッセージを送ってWorkerWを生成させる
        // 詳細：https://www.codeproject.com/Articles/856020/Draw-Behind-Desktop-Icons-in-Windows-plus
        SendMessageTimeout(progman, 0x052C, NULL, NULL, SMTO_NORMAL, 1000, NULL);

        // WorkerWは複数ある
        // アイコン裏の描画はデスクトップ管理ウィンドウの次のWorkerW
        HWND workerW = GetNextWindow(progman, GW_HWNDPREV);

        std::cout << "WorkerW:";

        if (workerW == NULL) std::cout << "NULL" << std::endl;
        else std::cout << workerW << std::endl;

        return workerW;
    }
}

