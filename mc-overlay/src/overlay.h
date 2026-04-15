#pragma once
#include <windows.h>

class Overlay {
public:
    bool Create(HINSTANCE hInst);
    void SyncWithGame();
    HWND GetHWND() const { return m_hwnd; }

private:
    HWND m_hwnd    = nullptr;
    HWND m_gameWnd = nullptr;

    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    HWND FindGameWindow();
};
