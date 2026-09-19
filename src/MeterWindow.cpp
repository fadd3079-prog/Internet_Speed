#include "MeterWindow.h"
#include "Taskbar.h"

#include <windowsx.h>
#include <string>
#include <cwchar>

namespace
{
    constexpr wchar_t WindowClassName[] = L"InternetSpeedMeterWindow";
    constexpr int WindowWidth = 190;
    constexpr int WindowHeight = 24;
    constexpr int TaskbarMargin = 14;
    constexpr int PaddingX = 8;
    constexpr int PaddingY = 4;

    constexpr UINT ExitCommand = 1;
    constexpr WORD FontResourceId = 101;
    constexpr COLORREF TransparentColor = RGB(0, 0, 0);

    std::wstring formatSpeed(double mbps)
    {
        if (mbps < 1.0)
        {
            wchar_t buffer[32]{};

            swprintf_s(
                buffer,
                L"%.0f Kbps",
                mbps * 1000.0
            );

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

        swprintf_s(
            buffer,
            L"%.2f Gbps",
            mbps / 1000.0
        );

        return buffer;
    }

    bool registerWindowClass(HINSTANCE instance)
    {
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = MeterWindow::WindowProc;
        wc.hInstance = instance;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.lpszClassName = WindowClassName;

        if (RegisterClassExW(&wc))
            return true;

        return GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
    }
}

MeterWindow::MeterWindow()
    : window_(nullptr),
      taskbar_(nullptr),
      instance_(nullptr),
      font_(nullptr),
      fontResource_(nullptr),
      text_{},
      width_(WindowWidth),
      height_(WindowHeight)
{
    swprintf_s(
        text_,
        L"\x2193 0 Mbps   \x2191 0 Mbps"
    );
}

MeterWindow::~MeterWindow()
{
    if (window_)
    {
        DestroyWindow(window_);
        window_ = nullptr;
    }

    if (font_)
    {
        DeleteObject(font_);
        font_ = nullptr;
    }

    if (fontResource_)
    {
        RemoveFontMemResourceEx(fontResource_);
        fontResource_ = nullptr;
    }
}

bool MeterWindow::create(
    HINSTANCE instance,
    HWND taskbar
)
{
    if (window_)
        return true;

    if (!instance || !taskbar)
        return false;

    instance_ = instance;
    taskbar_ = taskbar;

    if (!registerWindowClass(instance_))
        return false;

    window_ = CreateWindowExW(
        WS_EX_TOOLWINDOW |
        WS_EX_NOACTIVATE |
        WS_EX_TOPMOST |
        WS_EX_LAYERED,
        WindowClassName,
        L"",
        WS_POPUP,
        0,
        0,
        width_ + PaddingX * 2,
        height_ + PaddingY * 2,
        nullptr,
        nullptr,
        instance_,
        this
    );

    if (!window_)
        return false;

    SetWindowLongPtrW(
        window_,
        GWLP_HWNDPARENT,
        reinterpret_cast<LONG_PTR>(taskbar_)
    );

    if (!loadFont())
    {
        DestroyWindow(window_);
        window_ = nullptr;
        return false;
    }

    reposition();

    return true;
}

bool MeterWindow::loadFont()
{
    HRSRC resource =
        FindResourceW(
            instance_,
            MAKEINTRESOURCEW(FontResourceId),
            RT_RCDATA
        );

    if (!resource)
        return false;

    HGLOBAL loaded =
        LoadResource(
            instance_,
            resource
        );

    if (!loaded)
        return false;

    void* data =
        LockResource(loaded);

    if (!data)
        return false;

    const DWORD size =
        SizeofResource(
            instance_,
            resource
        );

    if (size == 0)
        return false;

    DWORD fontCount = 0;

    fontResource_ =
        AddFontMemResourceEx(
            data,
            size,
            nullptr,
            &fontCount
        );

    if (!fontResource_ || fontCount == 0)
    {
        fontResource_ = nullptr;
        return false;
    }

    const HDC hdc =
        GetDC(window_);

    if (!hdc)
    {
        RemoveFontMemResourceEx(fontResource_);
        fontResource_ = nullptr;
        return false;
    }

    const int dpi =
        GetDeviceCaps(
            hdc,
            LOGPIXELSY
        );

    ReleaseDC(
        window_,
        hdc
    );

    font_ =
        CreateFontW(
            -MulDiv(10, dpi, 72),
            0,
            0,
            0,
            FW_NORMAL,
            FALSE,
            FALSE,
            FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY,
            FIXED_PITCH | FF_MODERN,
            L"Space Mono"
        );

    if (!font_)
    {
        RemoveFontMemResourceEx(fontResource_);
        fontResource_ = nullptr;
        return false;
    }

    return true;
}

void MeterWindow::show()
{
    if (!window_)
        return;

    ShowWindow(
        window_,
        SW_SHOWNOACTIVATE
    );

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

    const std::wstring download =
        formatSpeed(downloadMbps);

    const std::wstring upload =
        formatSpeed(uploadMbps);

    swprintf_s(
        text_,
        L"\x2193 %ls   \x2191 %ls",
        download.c_str(),
        upload.c_str()
    );

    InvalidateRect(
        window_,
        nullptr,
        FALSE
    );

    UpdateWindow(window_);
}

void MeterWindow::reposition()
{
    if (!window_)
        return;

    HWND currentTaskbar =
        Taskbar::findTaskbar();

    if (!currentTaskbar)
        return;

    if (currentTaskbar != taskbar_)
    {
        taskbar_ = currentTaskbar;

        SetWindowLongPtrW(
            window_,
            GWLP_HWNDPARENT,
            reinterpret_cast<LONG_PTR>(taskbar_)
        );
    }

    const HWND tray =
        Taskbar::findTray(taskbar_);

    const POINT clientPosition =
        Taskbar::getMeterPosition(
            taskbar_,
            tray,
            width_,
            height_,
            TaskbarMargin
        );

    POINT screenPosition =
        clientPosition;

    ClientToScreen(
        taskbar_,
        &screenPosition
    );

    SetWindowPos(
        window_,
        HWND_TOPMOST,
        screenPosition.x - PaddingX,
        screenPosition.y - PaddingY,
        width_ + PaddingX * 2,
        height_ + PaddingY * 2,
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
            reinterpret_cast<const CREATESTRUCTW*>(
                lParam
            );

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
                GetWindowLongPtrW(
                    hwnd,
                    GWLP_USERDATA
                )
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
    case WM_MOUSEACTIVATE:
        return MA_NOACTIVATE;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
    {
        PAINTSTRUCT ps{};

        const HDC hdc =
            BeginPaint(
                hwnd,
                &ps
            );

        paint(hdc);

        EndPaint(
            hwnd,
            &ps
        );

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

        if (position.x == -1 &&
            position.y == -1)
        {
            GetCursorPos(&position);
        }

        showContextMenu(position);

        return 0;
    }

    case WM_DISPLAYCHANGE:
    case WM_SETTINGCHANGE:
        reposition();
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == ExitCommand)
        {
            DestroyWindow(hwnd);
            return 0;
        }

        break;

    case WM_NCDESTROY:
        SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            0
        );

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

void MeterWindow::paint(HDC)
{
    const int paddedWidth = width_ + PaddingX * 2;
    const int paddedHeight = height_ + PaddingY * 2;

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = paddedWidth;
    bmi.bmiHeader.biHeight = -paddedHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    const HDC screenDC = GetDC(nullptr);
    const HDC memDC = CreateCompatibleDC(screenDC);
    const HBITMAP bitmap = CreateDIBSection(
        screenDC, &bmi, DIB_RGB_COLORS,
        &bits, nullptr, 0);
    const HBITMAP oldBitmap =
        static_cast<HBITMAP>(
            SelectObject(memDC, bitmap));

    ZeroMemory(
        bits,
        paddedWidth * paddedHeight * 4
    );

    SetBkMode(memDC, TRANSPARENT);
    SelectObject(memDC, font_);
    SetTextColor(memDC, RGB(255, 255, 255));

    RECT textRect{
        PaddingX,
        PaddingY,
        paddedWidth - PaddingX,
        paddedHeight - PaddingY
    };

    DrawTextW(
        memDC,
        text_,
        -1,
        &textRect,
        DT_LEFT |
        DT_VCENTER |
        DT_SINGLELINE |
        DT_NOPREFIX
    );

    auto* pixels =
        static_cast<DWORD*>(bits);

    for (int i = 0;
         i < paddedWidth * paddedHeight;
         ++i)
    {
        if (pixels[i] & 0x00FFFFFF)
            pixels[i] =
                0xFF000000 |
                (pixels[i] & 0x00FFFFFF);
        else
            pixels[i] = 0x01000000;
    }

    POINT sourcePos{0, 0};
    SIZE windowSize{paddedWidth, paddedHeight};
    BLENDFUNCTION blend{};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;

    UpdateLayeredWindow(
        window_,
        screenDC,
        nullptr,
        &windowSize,
        memDC,
        &sourcePos,
        0,
        &blend,
        ULW_ALPHA
    );

    SelectObject(memDC, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(memDC);
    ReleaseDC(nullptr, screenDC);
}

void MeterWindow::showContextMenu(
    POINT position
)
{
    HMENU menu =
        CreatePopupMenu();

    if (!menu)
        return;

    AppendMenuW(
        menu,
        MF_STRING,
        ExitCommand,
        L"Exit"
    );

    SetForegroundWindow(
        window_
    );

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