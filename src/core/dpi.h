#pragma once

#include <windows.h>

inline int GetWindowDpi(HWND hwnd)
{
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32)
    {
        typedef UINT(WINAPI *fnGetDpiForWindow)(HWND);
        auto getDpi = reinterpret_cast<fnGetDpiForWindow>(GetProcAddress(hUser32, "GetDpiForWindow"));
        if (getDpi)
            return static_cast<int>(getDpi(hwnd));
    }
    HDC hdc = GetDC(hwnd);
    int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(hwnd, hdc);
    return dpi;
}

inline int ScaleByDpi(int value, int dpi)
{
    return MulDiv(value, dpi, 96);
}
