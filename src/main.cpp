#include "Network.h"
#include "MeterWindow.h"

#include <windows.h>

namespace
{
    constexpr UINT_PTR SpeedTimerId = 1;
    constexpr UINT_PTR RepositionTimerId = 2;

    constexpr UINT SpeedInterval = 1000;
    constexpr UINT RepositionInterval = 2000;

    void showError(const wchar_t* message)
    {
        const DWORD error = GetLastError();

        wchar_t buffer[256]{};

        wsprintfW(
            buffer,
            L"%ls\n\nWindows Error: %lu",
            message,
            error
        );

        MessageBoxW(
            nullptr,
            buffer,
            L"InternetSpeed Error",
            MB_OK | MB_ICONERROR
        );
    }
}

int WINAPI wWinMain(
    HINSTANCE instance,
    HINSTANCE,
    PWSTR,
    int
)
{
    NetworkMonitor network;
    MeterWindow meter;

    if (!meter.create(instance))
    {
        showError(L"MeterWindow::create() gagal.");
        return 1;
    }

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
        showError(L"Speed timer gagal dibuat.");
        return 1;
    }

    if (!SetTimer(
            meter.handle(),
            RepositionTimerId,
            RepositionInterval,
            nullptr))
    {
        showError(L"Reposition timer gagal dibuat.");
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
            if (message.wParam == SpeedTimerId)
            {
                if (network.update())
                {
                    const NetworkSpeed currentSpeed =
                        network.getSpeed();

                    meter.update(
                        currentSpeed.downloadMbps,
                        currentSpeed.uploadMbps
                    );
                }
            }
            else if (message.wParam == RepositionTimerId)
            {
                meter.reposition();
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
        RepositionTimerId
    );

    return static_cast<int>(message.wParam);
}