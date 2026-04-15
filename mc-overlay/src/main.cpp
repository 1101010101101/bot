#include "overlay.h"
#include "memory.h"
#include "render.h"
#include "types.h"
#include <thread>
#include <atomic>

// Global state — defined here, declared extern in types.h
GameData              g_data    = {};
std::atomic<bool>     g_running = true;

static void MemoryThread() {
    MCMemory mem;
    while (g_running && !mem.Attach("javaw.exe"))
        Sleep(1000);
    while (g_running) {
        mem.ReadGameData(g_data);
        Sleep(16);
    }
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    Overlay overlay;
    if (!overlay.Create(hInst)) {
        MessageBoxA(nullptr, "Failed to create overlay window", "Error", MB_OK);
        return 1;
    }

    // Stream-proof: invisible to OBS / Discord / any BitBlt capture
    // WDA_EXCLUDEFROMCAPTURE = 0x11, requires Win10 19041+
    if (!SetWindowDisplayAffinity(overlay.GetHWND(), 0x11))
        SetWindowDisplayAffinity(overlay.GetHWND(), WDA_MONITOR); // fallback: black box in stream

    DXRenderer renderer;
    if (!renderer.Init(overlay.GetHWND())) {
        MessageBoxA(nullptr, "Failed to init DX11", "Error", MB_OK);
        return 1;
    }

    std::thread memThread(MemoryThread);

    MSG msg = {};
    while (g_running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) g_running = false;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        overlay.SyncWithGame();
        renderer.BeginFrame();
        renderer.DrawESP(g_data);
        renderer.DrawHUD(g_data);
        renderer.EndFrame();
    }

    g_running = false;
    memThread.join();
    return 0;
}
