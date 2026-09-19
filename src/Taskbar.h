#pragma once

#include <windows.h>

namespace Taskbar
{
    HWND findTaskbar();
    HWND findTray(HWND taskbar);

    inline HWND getWindow()
    {
        return findTaskbar();
    }

    bool getRect(
        HWND window,
        RECT& rect
    );

    POINT getMeterPosition(
        HWND taskbar,
        HWND tray,
        int width,
        int height,
        int margin
    );
}