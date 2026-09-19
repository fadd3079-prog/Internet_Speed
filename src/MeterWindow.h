#pragma once

#include <windows.h>

class MeterWindow
{
public:
    MeterWindow();
    ~MeterWindow();

    bool create(HINSTANCE instance, HWND taskbar);
    void show();
    void update(double downloadMbps, double uploadMbps);
    void reposition();

    HWND handle() const;

    static LRESULT CALLBACK WindowProc(
        HWND hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );

private:
    LRESULT processMessage(
        HWND hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );

    void paint(HDC hdc);
    void showContextMenu(POINT position);

    HWND window_;
    HWND taskbar_;
    HINSTANCE instance_;
    HFONT font_;
    wchar_t text_[128];

    int width_;
    int height_;
};