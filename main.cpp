#include <windows.h>

#include <functional>
#include <iostream>
#include <string>

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

int main()
{
    const HWND workerW = getWorkerW();
    if (workerW == NULL) return -1;

    const std::string path = getWallpaper();
    if (path == "") return -1;

    //HDC hdc = GetDC(workerW);

    //HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
    //SelectObject(hdc, brush);
    //Rectangle(hdc, 0, 0, 1920, 1080);

    //HFONT font = CreateFont(
    //    20,                            // フォントの高さ(大きさ)。
    //    0,                             // フォントの幅。普通０。
    //    0,                             // 角度。０でＯＫ。
    //    0,                             // 同じく角度。これも０。
    //    FW_DONTCARE,                   // 文字の太さ。
    //    FALSE,                         // フォントがイタリックならTRUEを指定。
    //    FALSE,                         // 下線を引くならTRUE。
    //    FALSE,                         // 取り消し線を引くならTRUE。
    //    ANSI_CHARSET,                  // フォントの文字セット。このままでＯＫ。
    //    OUT_DEFAULT_PRECIS,            // 出力精度の設定。このままでＯＫ。
    //    CLIP_DEFAULT_PRECIS,           // クリッピング精度。このままでＯＫ。
    //    DRAFT_QUALITY,                 // フォントの出力品質。このままでＯＫ。
    //    DEFAULT_PITCH,                 // フォントのピッチとファミリを指定。このままでＯＫ。
    //    TEXT("Cascadia Mono SemiBold") // フォントのタイプフェイス名の指定。これは見たまんま。
    //);

    //SelectObject(hdc, font);

    //const wchar_t* text = L"Test";

    //SetBkMode(hdc, TRANSPARENT);

    //SetTextColor(hdc, RGB(19, 161, 14));
    //TextOutW(hdc, 100, 100, text, wcslen(text));

    //DeleteObject(brush);
    //DeleteObject(font);

    //ReleaseDC(workerW, hdc);

    //std::cin.get();

    setWallpaper(path);

    return 0;
}
