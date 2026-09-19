#include "Taskbar.h"

#include <shellapi.h>

namespace
{
    constexpr int VerticalOffset = 4;
    bool getTaskbarInfo(
        HWND taskbar,
        RECT &rect,
        UINT &edge)
    {
        APPBARDATA data{};
        data.cbSize = sizeof(data);

        if (SHAppBarMessage(
                ABM_GETTASKBARPOS,
                &data) == 0)
        {
            return false;
        }

        if (data.hWnd != taskbar)
            return false;

        rect = data.rc;
        edge = data.uEdge;

        return true;
    }
}

namespace Taskbar
{
    HWND findTaskbar()
    {
        return FindWindowW(
            L"Shell_TrayWnd",
            nullptr);
    }

    HWND findTray(HWND taskbar)
    {
        if (!taskbar)
            return nullptr;

        return FindWindowExW(
            taskbar,
            nullptr,
            L"TrayNotifyWnd",
            nullptr);
    }

    bool getRect(
        HWND window,
        RECT &rect)
    {
        if (!window)
            return false;

        return GetWindowRect(
            window,
            &rect);
    }

    POINT Taskbar::getMeterPosition(
        HWND taskbar,
        HWND tray,
        int width,
        int height,
        int margin)
    {
        RECT taskbarRect{};
        UINT edge = ABE_BOTTOM;

        if (!getTaskbarInfo(
                taskbar,
                taskbarRect,
                edge))
        {
            return {0, 0};
        }

        POINT screenPosition{};

        switch (edge)
        {
        case ABE_BOTTOM:
        {
            constexpr int HorizontalOffset = 16;
            constexpr int VerticalOffsetLocal = 4;

            screenPosition.x =
                taskbarRect.left +
                HorizontalOffset;

            screenPosition.y =
                taskbarRect.top +
                ((
                     taskbarRect.bottom -
                     taskbarRect.top -
                     height) /
                 2) +
                VerticalOffsetLocal;

            break;
        }

        case ABE_TOP:
        {
            constexpr int HorizontalOffset = 16;
            constexpr int VerticalOffsetLocal = 4;

            screenPosition.x =
                taskbarRect.left +
                HorizontalOffset;

            screenPosition.y =
                taskbarRect.top +
                ((
                     taskbarRect.bottom -
                     taskbarRect.top -
                     height) /
                 2) +
                VerticalOffsetLocal;

            break;
        }

        case ABE_LEFT:
        {
            screenPosition.x =
                taskbarRect.left +
                ((
                     taskbarRect.right -
                     taskbarRect.left -
                     width) /
                 2);

            screenPosition.y =
                taskbarRect.top +
                margin;

            break;
        }

        case ABE_RIGHT:
        {
            screenPosition.x =
                taskbarRect.left +
                ((
                     taskbarRect.right -
                     taskbarRect.left -
                     width) /
                 2);

            screenPosition.y =
                taskbarRect.top +
                margin;

            break;
        }

        default:
            return {0, 0};
        }

        return screenPosition;
    }
}