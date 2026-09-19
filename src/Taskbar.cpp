#include "Taskbar.h"

#include <shellapi.h>

namespace
{
    enum class TaskbarEdge
    {
        Left,
        Top,
        Right,
        Bottom
    };

    bool getTaskbarData(RECT& bounds, TaskbarEdge& edge)
    {
        APPBARDATA data{};
        data.cbSize = sizeof(data);

        if (SHAppBarMessage(ABM_GETTASKBARPOS, &data) == 0)
            return false;

        bounds = data.rc;

        switch (data.uEdge)
        {
        case ABE_LEFT:
            edge = TaskbarEdge::Left;
            break;

        case ABE_TOP:
            edge = TaskbarEdge::Top;
            break;

        case ABE_RIGHT:
            edge = TaskbarEdge::Right;
            break;

        default:
            edge = TaskbarEdge::Bottom;
            break;
        }

        return true;
    }

    bool getTrayBounds(RECT& bounds)
    {
        const HWND taskbar =
            FindWindowW(L"Shell_TrayWnd", nullptr);

        if (!taskbar)
            return false;

        const HWND tray =
            FindWindowExW(
                taskbar,
                nullptr,
                L"TrayNotifyWnd",
                nullptr
            );

        if (!tray)
            return false;

        return GetWindowRect(tray, &bounds);
    }

    bool getMonitorWorkArea(const RECT& taskbar, RECT& workArea)
    {
        POINT center{
            (taskbar.left + taskbar.right) / 2,
            (taskbar.top + taskbar.bottom) / 2
        };

        const HMONITOR monitor =
            MonitorFromPoint(
                center,
                MONITOR_DEFAULTTONEAREST
            );

        if (!monitor)
            return false;

        MONITORINFO info{};
        info.cbSize = sizeof(info);

        if (!GetMonitorInfoW(monitor, &info))
            return false;

        workArea = info.rcWork;
        return true;
    }
}

namespace Taskbar
{
    bool getBounds(RECT& bounds)
    {
        TaskbarEdge edge{};
        return getTaskbarData(bounds, edge);
    }

    bool getWorkArea(RECT& workArea)
    {
        RECT taskbar{};
        TaskbarEdge edge{};

        if (!getTaskbarData(taskbar, edge))
            return false;

        return getMonitorWorkArea(taskbar, workArea);
    }

    POINT getMeterPosition(
        int width,
        int height,
        int margin
    )
    {
        RECT taskbar{};
        TaskbarEdge edge{};

        if (!getTaskbarData(taskbar, edge))
        {
            RECT workArea{};

            if (getWorkArea(workArea))
            {
                return {
                    workArea.right - width - margin,
                    workArea.bottom - height - margin
                };
            }

            return {
                margin,
                margin
            };
        }

        RECT tray{};
        const bool trayAvailable = getTrayBounds(tray);

        switch (edge)
        {
        case TaskbarEdge::Bottom:
            return {
                trayAvailable
                    ? tray.left - width - margin
                    : taskbar.right - width - margin,
                taskbar.bottom - height
            };

        case TaskbarEdge::Top:
            return {
                trayAvailable
                    ? tray.left - width - margin
                    : taskbar.right - width - margin,
                taskbar.top
            };

        case TaskbarEdge::Right:
            return {
                taskbar.right - width,
                trayAvailable
                    ? tray.top - height - margin
                    : taskbar.bottom - height - margin
            };

        case TaskbarEdge::Left:
            return {
                taskbar.left,
                trayAvailable
                    ? tray.top - height - margin
                    : taskbar.bottom - height - margin
            };
        }

        return {
            taskbar.right - width - margin,
            taskbar.bottom - height
        };
    }
}