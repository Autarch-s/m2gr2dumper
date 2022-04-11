// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

typedef int(__stdcall* _GrannyReadEntireFileFromMemory)(int size, void const* data);
_GrannyReadEntireFileFromMemory GrannyReadEntireFileFromMemory;

void dump(int size, void const* data) {
    if (size == 0 || data == nullptr)
        return;

    const char* path = "dump/";
    if (_access(path, 0) == -1) {
        CreateDirectoryA(path, nullptr);
    }

    {
        std::string filename = path + std::to_string(size) + ".gr2";
        if ((_access(filename.c_str(), 0)) != -1) {
            printf("%s exists\n", filename.c_str());
        }
        else {
            printf("dump %s (%d, %p)\n", filename.c_str(), size, data);
            FILE* fp = nullptr;
            fopen_s(&fp, filename.c_str(), "wb");
            if (fp) {
                fwrite(data, size, 1, fp);
                fclose(fp);
            }
        }
    }
}

int __stdcall GrannyReadEntireFileFromMemoryHook(int size, void const* data) {
    dump(size, data);
    return GrannyReadEntireFileFromMemory(size, data);
}

void Start() {
    if (!AllocConsole()) {
        abort();
    }

    FILE* dummy;
    freopen_s(&dummy, "CONIN$", "r", stdin);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
    freopen_s(&dummy, "CONOUT$", "w", stdout);

    HMODULE hModule = GetModuleHandleA("granny2.dll");
    if (hModule == nullptr) {
        printf("Granny2DLL not found\n");
    }
    else {
        FARPROC func = GetProcAddress(hModule, "_GrannyReadEntireFileFromMemory@8");
        if (func == nullptr)
        {
            printf("GrannyReadEntireFileFromMemory not found\n");
        }
        else {
            printf("GrannyReadEntireFileFromMemory %p\n", func);
            GrannyReadEntireFileFromMemory = (_GrannyReadEntireFileFromMemory)DetourFunction((PBYTE)func, (PBYTE)GrannyReadEntireFileFromMemoryHook);
            printf("GrannyReadEntireFileFromMemoryHook %p\n", GrannyReadEntireFileFromMemory);
        }
    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)&Start, nullptr, 0, nullptr);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

