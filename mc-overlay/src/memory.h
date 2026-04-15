#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <vector>
#include "types.h"

class MCMemory {
public:
    bool Attach(const char* procName);
    void ReadGameData(GameData& out);
    void Detach();
    ~MCMemory() { Detach(); }

    template<typename T>
    T Read(uintptr_t addr) {
        T val{};
        ReadProcessMemory(m_proc, (LPCVOID)addr, &val, sizeof(T), nullptr);
        return val;
    }

    uintptr_t ReadPtr(uintptr_t addr) { return Read<uintptr_t>(addr); }

private:
    HANDLE    m_proc       = nullptr;
    DWORD     m_pid        = 0;
    uintptr_t m_moduleBase = 0;

    uintptr_t m_entityPlayerAddr = 0;
    uintptr_t m_worldAddr        = 0;

    DWORD     GetPID(const char* name);
    uintptr_t GetModuleBase(DWORD pid, const char* modName);
    uintptr_t AOBScan(const char* pattern, const char* mask,
                      uintptr_t start, size_t size);
    void      ResolveAddresses();
};
