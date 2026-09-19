#include "Network.h"
#include "MeterWindow.h"
#include "Taskbar.h"

#include <windows.h>

namespace
{
    constexpr UINT_PTR SpeedTimerId = 1;
    constexpr UINT_PTR TaskbarTimerId = 2;

    constexpr UINT SpeedInterval = 1000;
    constexpr UINT TaskbarInterval = 2000;
}

int WINAPI wWinMain(
    HINSTANCE instance,
    HINSTANCE,
    PWSTR,
    int
)
{
    HWND taskbar = Taskbar::getWindow();

    if (!taskbar)
        return 1;

    NetworkMonitor network;
    MeterWindow meter;

    if (!meter.create(instance, taskbar))
        return 1;

    meter.show();

    network.update();

    const NetworkSpeed speed = network.getSpeed();

    meter.update(
        speed.downloadMbps,
        speed.uploadMbps
    );

    if (!SetTimer(
            meter.handle(),
            SpeedTimerId,
            SpeedInterval,
            nullptr))
    {
        return 1;
    }

    if (!SetTimer(
            meter.handle(),
            TaskbarTimerId,
            TaskbarInterval,
            nullptr))
    {
        KillTimer(
            meter.handle(),
            SpeedTimerId
        );

        return 1;
    }

    MSG message{};

    while (GetMessageW(
        &message,
        nullptr,
        0,
        0) > 0)
    {
        if (message.message == WM_TIMER &&
            message.hwnd == meter.handle())
        {
            switch (message.wParam)
            {
            case SpeedTimerId:
                if (network.update())
                {
                    const NetworkSpeed current =
                        network.getSpeed();

                    meter.update(
                        current.downloadMbps,
                        current.uploadMbps
                    );
                }
                break;

            case TaskbarTimerId:
                meter.reposition();
                break;
            }

            continue;
        }

        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    KillTimer(
        meter.handle(),
        SpeedTimerId
    );

    KillTimer(
        meter.handle(),
        TaskbarTimerId
    );

    return static_cast<int>(message.wParam);
}