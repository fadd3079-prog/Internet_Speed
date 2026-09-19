#include "MeterWindow.h"
#include "Taskbar.h"

#include <windowsx.h>
#include <string>
#include <cwchar>

namespace
{
    constexpr wchar_t WindowClassName[] = L"InternetSpeedMeterWindow";
    constexpr wchar_t WindowTitle[] = L"Internet Speed";

    constexpr int WindowWidth = 180;
    constexpr int WindowHeight = 24;
    constexpr int TaskbarMargin = 4;

    constexpr UINT_PTR ExitCommand = 1;

    std::wstring formatSpeed(double mbps)
    {
        if (mbps < 1.0)
        {
            wchar_t buffer[32]{};
            swprintf_s(buffer, L"%.0f Kbps", mbps * 1000.0);
            return buffer;
        }

        if (mbps < 1000.0)
        {
            wchar_t buffer[32]{};
            swprintf_s(
                buffer,
                mbps < 100.0 ? L"%.1f Mbps" : L"%.0f Mbps",
                mbps
            );
            return buffer;
        }

        wchar_t buffer[32]{};
        swprintf_s(buffer, L"%.2f Gbps", mbps / 1000.0);
        return buffer;
    }

    void registerWindowClass(HINSTANCE instance)
    {
        static bool registered = false;

        if (registered)
            return;

        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.hInstance = instance;
        wc.lpfnWndProc = MeterWindow::WindowProc;
        wc.lpszClassName = WindowClassName;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);

        RegisterClassExW(&wc);
        registered = true;
    }
}

MeterWindow::MeterWindow()
    : window_(nullptr),
      instance_(nullptr),
      font_(nullptr),
      text_{},
      width_(WindowWidth),
      height_(WindowHeight)
{
    wcscpy_s(text_, L"↓ 0 Mbps   ↑ 0 Mbps");
}

MeterWindow::~MeterWindow()
{
    if (font_)
        DeleteObject(font_);

    if (window_)
        DestroyWindow(window_);
}

bool MeterWindow::create(HINSTANCE instance)
{
    if (window_)
        return true;

    instance_ = instance;

    registerWindowClass(instance_);

    window_ = CreateWindowExW(
        WS_EX_TOOLWINDOW |
        WS_EX_NOACTIVATE |
        WS_EX_TOPMOST,
        WindowClassName,
        WindowTitle,
        WS_POPUP,
        0,
        0,
        width_,
        height_,
        nullptr,
        nullptr,
        instance_,
        this
    );

    if (!window_)
        return false;

    const HDC hdc = GetDC(window_);

    if (!hdc)
        return false;

    const int dpi = GetDeviceCaps(hdc, LOGPIXELSY);

    ReleaseDC(window_, hdc);

    font_ = CreateFontW(
        -MulDiv(9, dpi, 72),
        0,
        0,
        0,
        FW_SEMIBOLD,
        FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS,
        L"Segoe UI"
    );

    if (!font_)
    {
        DestroyWindow(window_);
        window_ = nullptr;
        return false;
    }

    reposition();

    return true;
}

void MeterWindow::show()
{
    if (!window_)
        return;

    ShowWindow(window_, SW_SHOWNOACTIVATE);

    SetWindowPos(
        window_,
        HWND_TOPMOST,
        0,
        0,
        0,
        0,
        SWP_NOMOVE |
        SWP_NOSIZE |
        SWP_NOACTIVATE |
        SWP_SHOWWINDOW
    );

    UpdateWindow(window_);
}

void MeterWindow::update(
    double downloadMbps,
    double uploadMbps
)
{
    if (!window_)
        return;

    const std::wstring download = formatSpeed(downloadMbps);
    const std::wstring upload = formatSpeed(uploadMbps);

    swprintf_s(
        text_,
        L"↓ %ls   ↑ %ls",
        download.c_str(),
        upload.c_str()
    );

    InvalidateRect(window_, nullptr, FALSE);
}

void MeterWindow::reposition()
{
    if (!window_)
        return;

    const POINT position =
        Taskbar::getMeterPosition(
            width_,
            height_,
            TaskbarMargin
        );

    SetWindowPos(
        window_,
        HWND_TOPMOST,
        position.x,
        position.y,
        width_,
        height_,
        SWP_NOACTIVATE |
        SWP_SHOWWINDOW
    );
}

HWND MeterWindow::handle() const
{
    return window_;
}

LRESULT CALLBACK MeterWindow::WindowProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    MeterWindow* meter = nullptr;

    if (message == WM_NCCREATE)
    {
        const auto* createStruct =
            reinterpret_cast<const CREATESTRUCTW*>(lParam);

        meter =
            static_cast<MeterWindow*>(
                createStruct->lpCreateParams
            );

        SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(meter)
        );
    }
    else
    {
        meter =
            reinterpret_cast<MeterWindow*>(
                GetWindowLongPtrW(hwnd, GWLP_USERDATA)
            );
    }

    if (meter)
    {
        return meter->processMessage(
            hwnd,
            message,
            wParam,
            lParam
        );
    }

    return DefWindowProcW(
        hwnd,
        message,
        wParam,
        lParam
    );
}

LRESULT MeterWindow::processMessage(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (message)
    {
    case WM_NCCREATE:
        return TRUE;

    case WM_MOUSEACTIVATE:
        return MA_NOACTIVATE;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
    {
        PAINTSTRUCT ps{};
        const HDC hdc = BeginPaint(hwnd, &ps);

        paint(hdc);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_RBUTTONUP:
    {
        POINT position{};
        GetCursorPos(&position);

        showContextMenu(position);
        return 0;
    }

    case WM_CONTEXTMENU:
    {
        POINT position{
            GET_X_LPARAM(lParam),
            GET_Y_LPARAM(lParam)
        };

        if (position.x == -1 && position.y == -1)
            GetCursorPos(&position);

        showContextMenu(position);
        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == ExitCommand)
        {
            DestroyWindow(hwnd);
            return 0;
        }
        break;

    case WM_NCDESTROY:
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(
        hwnd,
        message,
        wParam,
        lParam
    );
}

void MeterWindow::paint(HDC hdc)
{
    SetBkMode(hdc, TRANSPARENT);

    RECT rect{};
    GetClientRect(window_, &rect);

    SelectObject(hdc, font_);

    RECT shadowRect = rect;
    OffsetRect(&shadowRect, 1, 1);

    SetTextColor(hdc, RGB(0, 0, 0));

    DrawTextW(
        hdc,
        text_,
        -1,
        &shadowRect,
        DT_CENTER |
        DT_VCENTER |
        DT_SINGLELINE |
        DT_NOPREFIX
    );

    SetTextColor(hdc, RGB(255, 255, 255));

    DrawTextW(
        hdc,
        text_,
        -1,
        &rect,
        DT_CENTER |
        DT_VCENTER |
        DT_SINGLELINE |
        DT_NOPREFIX
    );
}

void MeterWindow::showContextMenu(POINT position)
{
    HMENU menu = CreatePopupMenu();

    if (!menu)
        return;

    AppendMenuW(
        menu,
        MF_STRING,
        ExitCommand,
        L"Exit"
    );

    SetForegroundWindow(window_);

    const UINT command =
        TrackPopupMenuEx(
            menu,
            TPM_RIGHTBUTTON |
            TPM_NOANIMATION |
            TPM_RETURNCMD,
            position.x,
            position.y,
            window_,
            nullptr
        );

    if (command == ExitCommand)
        DestroyWindow(window_);

    DestroyMenu(menu);
}