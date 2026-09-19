#pragma once

#include <windows.h>

namespace Taskbar
{
    HWND getWindow();

    bool getBounds(
        HWND taskbar,
        RECT& bounds
    );

    POINT getMeterPosition(
        HWND taskbar,
        int width,
        int height,
        int margin
    );
}