#include "memory.h"
#include <Psapi.h>
#include <cstring>
#pragma comment(lib, "Psapi.lib")

// ---- Offsets (replace with real values from CE pointer scan) ----
#define OFF_ENTITY_POS_X   0x30   // double
#define OFF_ENTITY_POS_Y   0x38   // double
#define OFF_ENTITY_POS_Z   0x40   // double
#define OFF_ENTITY_HEALTH  0x100  // float
#define OFF_ENTITY_YAW     0xC8   // float
#define OFF_ENTITY_PITCH   0xCC   // float
#define OFF_ENTITY_NAME    0x68   // String object ptr
#define OFF_STRING_VALUE   0x10   // char[] inside String
// -----------------------------------------------------------------

DWORD MCMemory::GetPID(const char* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32 pe = { sizeof(pe) };
    DWORD pid = 0;
    if (Process32First(snap, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, name) == 0) { pid = pe.th32ProcessID; break; }
        } while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);
    return pid;
}

uintptr_t MCMemory::GetModuleBase(DWORD pid, const char* modName) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    MODULEENTRY32 me = { sizeof(me) };
    uintptr_t base = 0;
    if (Module32First(snap, &me)) {
        do {
            if (_stricmp(me.szModule, modName) == 0) { base = (uintptr_t)me.modBaseAddr; break; }
        } while (Module32Next(snap, &me));
    }
    CloseHandle(snap);
    return base;
}

bool MCMemory::Attach(const char* procName) {
    m_pid = GetPID(procName);
    if (!m_pid) return false;
    m_proc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, m_pid);
    if (!m_proc) return false;
    m_moduleBase = GetModuleBase(m_pid, "jvm.dll");
    ResolveAddresses();
    return true;
}

void MCMemory::Detach() {
    if (m_proc) { CloseHandle(m_proc); m_proc = nullptr; }
}

uintptr_t MCMemory::AOBScan(const char* pattern, const char* mask,
                             uintptr_t start, size_t scanSize) {
    size_t patLen = strlen(mask);
    std::vector<BYTE> buf(4096);
    uintptr_t addr = start;
    size_t remaining = scanSize;
    while (remaining > 0) {
        SIZE_T toRead = (buf.size() < remaining) ? buf.size() : remaining;
        SIZE_T bytesRead = 0;
        if (!ReadProcessMemory(m_proc, (LPCVOID)addr, buf.data(), toRead, &bytesRead) || bytesRead == 0) {
            addr += 4096; remaining = (remaining > 4096) ? remaining - 4096 : 0; continue;
        }
        for (size_t i = 0; i + patLen <= bytesRead; i++) {
            bool found = true;
            for (size_t j = 0; j < patLen; j++) {
                if (mask[j] == 'x' && buf[i + j] != (BYTE)pattern[j]) { found = false; break; }
            }
            if (found) return addr + i;
        }
        addr += bytesRead;
        remaining = (remaining > bytesRead) ? remaining - bytesRead : 0;
    }
    return 0;
}

void MCMemory::ResolveAddresses() {
    // TODO: fill from CE pointer scan for your specific client build
    m_entityPlayerAddr = 0;
    m_worldAddr        = 0;
}

void MCMemory::ReadGameData(GameData& out) {
    if (!m_entityPlayerAddr) return;

    out.localX = (float)Read<double>(m_entityPlayerAddr + OFF_ENTITY_POS_X);
    out.localY = (float)Read<double>(m_entityPlayerAddr + OFF_ENTITY_POS_Y);
    out.localZ = (float)Read<double>(m_entityPlayerAddr + OFF_ENTITY_POS_Z);
    out.health = Read<float>(m_entityPlayerAddr + OFF_ENTITY_HEALTH);
    out.yaw    = Read<float>(m_entityPlayerAddr + OFF_ENTITY_YAW);
    out.pitch  = Read<float>(m_entityPlayerAddr + OFF_ENTITY_PITCH);

    if (!m_worldAddr) return;

    uintptr_t listObj     = ReadPtr(m_worldAddr + 0x50);
    uintptr_t elementData = ReadPtr(listObj + 0x18);
    int size              = Read<int>(listObj + 0x14);
    if (size < 0) size = 0;
    if (size > 256) size = 256;
    out.entityCount = 0;

    for (int i = 0; i < size; i++) {
        uintptr_t entPtr = ReadPtr(elementData + 0x10 + (uintptr_t)i * 8);
        if (!entPtr) continue;

        auto& e  = out.entities[out.entityCount];
        e.x      = (float)Read<double>(entPtr + OFF_ENTITY_POS_X);
        e.y      = (float)Read<double>(entPtr + OFF_ENTITY_POS_Y);
        e.z      = (float)Read<double>(entPtr + OFF_ENTITY_POS_Z);
        e.health = Read<float>(entPtr + OFF_ENTITY_HEALTH);

        uintptr_t nameStr = ReadPtr(entPtr + OFF_ENTITY_NAME);
        uintptr_t charArr = ReadPtr(nameStr + OFF_STRING_VALUE);
        int nameLen       = Read<int>(charArr + 0x10);
        if (nameLen < 0) nameLen = 0;
        if (nameLen > 63) nameLen = 63;
        wchar_t wname[64] = {};
        ReadProcessMemory(m_proc, (LPCVOID)(charArr + 0x14), wname, nameLen * 2, nullptr);
        WideCharToMultiByte(CP_UTF8, 0, wname, nameLen, e.name, sizeof(e.name) - 1, nullptr, nullptr);
        e.name[63] = '\0';
        e.isPlayer = (e.name[0] != '\0'); // refine with class vtable check
        out.entityCount++;
    }
}
