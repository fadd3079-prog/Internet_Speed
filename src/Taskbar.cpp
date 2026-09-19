#include "Taskbar.h"

#include <shellapi.h>

namespace
{
    bool getTaskbarRect(HWND taskbar, RECT& rect)
    {
        APPBARDATA data{};
        data.cbSize = sizeof(data);

        if (SHAppBarMessage(
                ABM_GETTASKBARPOS,
                &data) == 0)
        {
            return false;
        }

        if (taskbar != data.hWnd)
            return false;

        rect = data.rc;
        return true;
    }

    bool getTaskbarEdge(
        HWND taskbar,
        UINT& edge)
    {
        APPBARDATA data{};
        data.cbSize = sizeof(data);

        if (SHAppBarMessage(
                ABM_GETTASKBARPOS,
                &data) == 0)
        {
            return false;
        }

        if (taskbar != data.hWnd)
            return false;

        edge = data.uEdge;
        return true;
    }

    bool getTrayRect(
        HWND taskbar,
        RECT& rect)
    {
        const HWND tray =
            FindWindowExW(
                taskbar,
                nullptr,
                L"TrayNotifyWnd",
                nullptr
            );

        if (!tray)
            return false;

        return GetWindowRect(tray, &rect);
    }
}

namespace Taskbar
{
    HWND getWindow()
    {
        return FindWindowW(
            L"Shell_TrayWnd",
            nullptr
        );
    }

    bool getBounds(
        HWND taskbar,
        RECT& bounds
    )
    {
        if (!taskbar)
            return false;

        return getTaskbarRect(
            taskbar,
            bounds
        );
    }

    POINT getMeterPosition(
        HWND taskbar,
        int width,
        int height,
        int margin
    )
    {
        RECT taskbarRect{};

        if (!getBounds(
                taskbar,
                taskbarRect))
        {
            return {0, 0};
        }

        UINT edge = ABE_BOTTOM;

        if (!getTaskbarEdge(
                taskbar,
                edge))
        {
            return {0, 0};
        }

        RECT trayRect{};
        const bool hasTray =
            getTrayRect(
                taskbar,
                trayRect
            );

        POINT position{};

        switch (edge)
        {
        case ABE_TOP:
        case ABE_BOTTOM:
        {
            const int x =
                hasTray
                    ? trayRect.left - width - margin
                    : taskbarRect.right - width - margin;

            const int y =
                taskbarRect.top +
                ((taskbarRect.bottom -
                  taskbarRect.top -
                  height) / 2);

            position = {x, y};
            break;
        }

        case ABE_LEFT:
        case ABE_RIGHT:
        {
            const int x =
                taskbarRect.left +
                ((taskbarRect.right -
                  taskbarRect.left -
                  width) / 2);

            const int y =
                hasTray
                    ? trayRect.top - height - margin
                    : taskbarRect.bottom - height - margin;

            position = {x, y};
            break;
        }

        default:
            position = {
                taskbarRect.right - width - margin,
                taskbarRect.bottom - height
            };
            break;
        }

        ScreenToClient(
            taskbar,
            &position
        );

        return position;
    }
}