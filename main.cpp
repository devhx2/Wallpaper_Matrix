#include "utility.hpp"

#include <windows.h>

#include <functional>
#include <iostream>
#include <string>

int main()
{
    const HWND workerW = utility::getWorkerW();
    if (workerW == NULL) return -1;

    const std::string path = utility::getWallpaper();
    if (path == "") return -1;

    //// 壁紙のパスを取得
    //char path[MAX_PATH]{};
    //SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, path, 0);
    //std::cout << "Path:" << path << std::endl;

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

    //SystemParametersInfo(SPI_SETDESKWALLPAPER, NULL, path, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

    //std::cin.get();

    utility::setWallpaper(path);

    return 0;
}
