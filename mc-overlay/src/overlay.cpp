#include "overlay.h"
#include <cstring>

HWND Overlay::FindGameWindow() {
    HWND hwnd = GetTopWindow(nullptr);
    while (hwnd) {
        char title[256] = {};
        GetWindowTextA(hwnd, title, sizeof(title));
        if (strstr(title, "Minecraft") && IsWindowVisible(hwnd))
            return hwnd;
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }
    return nullptr;
}

LRESULT CALLBACK Overlay::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

bool Overlay::Create(HINSTANCE hInst) {
    WNDCLASSEXA wc    = {};
    wc.cbSize         = sizeof(wc);
    wc.style          = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc    = WndProc;
    wc.hInstance      = hInst;
    wc.lpszClassName  = "MCOverlay";
    wc.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
    if (!RegisterClassExA(&wc)) return false;

    DWORD exStyle = WS_EX_LAYERED
                  | WS_EX_TRANSPARENT
                  | WS_EX_TOPMOST
                  | WS_EX_TOOLWINDOW;

    m_hwnd = CreateWindowExA(
        exStyle,
        "MCOverlay", "",
        WS_POPUP,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        nullptr, nullptr, hInst, nullptr
    );
    if (!m_hwnd) return false;

    // Black = transparent via ColorKey
    SetLayeredWindowAttributes(m_hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    return true;
}

void Overlay::SyncWithGame() {
    if (!m_gameWnd || !IsWindow(m_gameWnd))
        m_gameWnd = FindGameWindow();
    if (!m_gameWnd) return;

    RECT r;
    GetWindowRect(m_gameWnd, &r);
    SetWindowPos(
        m_hwnd, HWND_TOPMOST,
        r.left, r.top,
        r.right  - r.left,
        r.bottom - r.top,
        SWP_NOACTIVATE
    );
}
