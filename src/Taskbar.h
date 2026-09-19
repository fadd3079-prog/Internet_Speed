#pragma once

#include <windows.h>

namespace Taskbar
{
    bool getBounds(RECT& bounds);
    bool getWorkArea(RECT& workArea);
    POINT getMeterPosition(int width, int height, int margin);
}