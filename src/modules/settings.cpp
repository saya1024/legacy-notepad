/*
   ▄████████  ▄██████▄     ▄████████  ▄█        ▄██████▄   ▄██████▄     ▄███████▄
  ███    ███ ███    ███   ███    ███ ███       ███    ███ ███    ███   ███    ███
  ███    █▀  ███    ███   ███    ███ ███       ███    ███ ███    ███   ███    ███
 ▄███▄▄▄     ███    ███  ▄███▄▄▄▄██▀ ███       ███    ███ ███    ███   ███    ███
▀▀███▀▀▀     ███    ███ ▀▀███▀▀▀▀▀   ███       ███    ███ ███    ███ ▀█████████▀
  ███        ███    ███ ▀███████████ ███       ███    ███ ███    ███   ███
  ███        ███    ███   ███    ███ ███▌    ▄ ███    ███ ███    ███   ███
  ███         ▀██████▀    ███    ███ █████▄▄██  ▀██████▀   ▀██████▀   ▄████▀
                          ███    ███ ▀

  Settings management for persisting user preferences via INI file.
  Handles font settings, always on top, window size and position storage and retrieval.
*/

#include "settings.h"
#include "core/globals.h"
#include "core/types.h"
#include <windows.h>

#define SETTINGS_INI L"Notepad.ini"

#define FONT_SECTION L"Font"
#define FONT_NAME_KEY L"Name"
#define FONT_SIZE_KEY L"Size"
#define FONT_WEIGHT_KEY L"Weight"
#define FONT_ITALIC_KEY L"Italic"
#define FONT_UNDERLINE_KEY L"Underline"

#define WINDOW_SECTION L"Window"
#define WINDOW_X_KEY L"X"
#define WINDOW_Y_KEY L"Y"
#define WINDOW_WIDTH_KEY L"Width"
#define WINDOW_HEIGHT_KEY L"Height"

#define SETTINGS_SECTION L"Settings"
#define ALWAYS_ON_TOP_KEY L"AlwaysOnTop"

#define MIN_FONT_SIZE 8
#define MAX_FONT_SIZE 72

static std::wstring GetIniPath()
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    wchar_t *ext = wcsrchr(path, L'.');
    if (ext)
        wcscpy_s(ext, MAX_PATH - (ext - path), L".ini");
    return path;
}

void LoadFontSettings()
{
    std::wstring iniPath = GetIniPath();

    wchar_t fontName[LF_FACESIZE] = {0};
    GetPrivateProfileStringW(FONT_SECTION, FONT_NAME_KEY, L"Consolas", fontName, LF_FACESIZE, iniPath.c_str());
    g_state.fontName = fontName;

    int fontSize = static_cast<int>(GetPrivateProfileIntW(FONT_SECTION, FONT_SIZE_KEY, 16, iniPath.c_str()));
    if (fontSize >= MIN_FONT_SIZE && fontSize <= MAX_FONT_SIZE)
    {
        g_state.fontSize = fontSize;
    }

    int weight = static_cast<int>(GetPrivateProfileIntW(FONT_SECTION, FONT_WEIGHT_KEY, FW_NORMAL, iniPath.c_str()));
    g_state.fontWeight = weight;

    int italic = static_cast<int>(GetPrivateProfileIntW(FONT_SECTION, FONT_ITALIC_KEY, 0, iniPath.c_str()));
    g_state.fontItalic = (italic != 0);

    int underline = static_cast<int>(GetPrivateProfileIntW(FONT_SECTION, FONT_UNDERLINE_KEY, 0, iniPath.c_str()));
    g_state.fontUnderline = (underline != 0);

    int top = static_cast<int>(GetPrivateProfileIntW(SETTINGS_SECTION, ALWAYS_ON_TOP_KEY, 0, iniPath.c_str()));
    g_state.alwaysOnTop = (top != 0);
}

void SaveFontSettings()
{
    std::wstring iniPath = GetIniPath();

    WritePrivateProfileStringW(FONT_SECTION, FONT_NAME_KEY, g_state.fontName.c_str(), iniPath.c_str());

    wchar_t buf[32];
    swprintf(buf, 32, L"%d", g_state.fontSize);
    WritePrivateProfileStringW(FONT_SECTION, FONT_SIZE_KEY, buf, iniPath.c_str());

    swprintf(buf, 32, L"%d", g_state.fontWeight);
    WritePrivateProfileStringW(FONT_SECTION, FONT_WEIGHT_KEY, buf, iniPath.c_str());

    swprintf(buf, 32, L"%d", g_state.fontItalic ? 1 : 0);
    WritePrivateProfileStringW(FONT_SECTION, FONT_ITALIC_KEY, buf, iniPath.c_str());

    swprintf(buf, 32, L"%d", g_state.fontUnderline ? 1 : 0);
    WritePrivateProfileStringW(FONT_SECTION, FONT_UNDERLINE_KEY, buf, iniPath.c_str());

    swprintf(buf, 32, L"%d", g_state.alwaysOnTop ? 1 : 0);
    WritePrivateProfileStringW(SETTINGS_SECTION, ALWAYS_ON_TOP_KEY, buf, iniPath.c_str());
}

void LoadWindowSettings()
{
    std::wstring iniPath = GetIniPath();

    wchar_t buf[32];

    GetPrivateProfileStringW(WINDOW_SECTION, WINDOW_X_KEY, L"", buf, 32, iniPath.c_str());
    if (buf[0])
    {
        int x = static_cast<int>(wcstol(buf, nullptr, 10));
        g_state.windowX = x;
    }

    GetPrivateProfileStringW(WINDOW_SECTION, WINDOW_Y_KEY, L"", buf, 32, iniPath.c_str());
    if (buf[0])
    {
        int y = static_cast<int>(wcstol(buf, nullptr, 10));
        g_state.windowY = y;
    }

    int width = static_cast<int>(GetPrivateProfileIntW(WINDOW_SECTION, WINDOW_WIDTH_KEY, 0, iniPath.c_str()));
    if (width > 0)
    {
        g_state.windowWidth = width;
    }

    int height = static_cast<int>(GetPrivateProfileIntW(WINDOW_SECTION, WINDOW_HEIGHT_KEY, 0, iniPath.c_str()));
    if (height > 0)
    {
        g_state.windowHeight = height;
    }

    if (g_state.windowX != CW_USEDEFAULT && g_state.windowY != CW_USEDEFAULT)
    {
        RECT rc = {g_state.windowX, g_state.windowY, g_state.windowX + g_state.windowWidth, g_state.windowY + g_state.windowHeight};
        HMONITOR hMonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONULL);
        if (!hMonitor)
        {
            g_state.windowX = CW_USEDEFAULT;
            g_state.windowY = CW_USEDEFAULT;
        }
    }
}

void SaveWindowSettings()
{
    std::wstring iniPath = GetIniPath();

    wchar_t buf[32];

    swprintf(buf, 32, L"%d", g_state.windowX);
    WritePrivateProfileStringW(WINDOW_SECTION, WINDOW_X_KEY, buf, iniPath.c_str());

    swprintf(buf, 32, L"%d", g_state.windowY);
    WritePrivateProfileStringW(WINDOW_SECTION, WINDOW_Y_KEY, buf, iniPath.c_str());

    swprintf(buf, 32, L"%d", g_state.windowWidth);
    WritePrivateProfileStringW(WINDOW_SECTION, WINDOW_WIDTH_KEY, buf, iniPath.c_str());

    swprintf(buf, 32, L"%d", g_state.windowHeight);
    WritePrivateProfileStringW(WINDOW_SECTION, WINDOW_HEIGHT_KEY, buf, iniPath.c_str());
}
