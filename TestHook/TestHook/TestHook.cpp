#include <windows.h>
#include <iostream>
#include "MinHook.h"

typedef HANDLE(WINAPI* CreateFileW_t)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
CreateFileW_t fpCreateFileW = nullptr;

// Hook callback function
HANDLE WINAPI HookedCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile) {

    std::wcout << L"[BEService] CreateFileW called with file: " << lpFileName << std::endl;
    return fpCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
        dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        MH_Initialize();
        MH_CreateHook(&CreateFileW, &HookedCreateFileW, reinterpret_cast<LPVOID*>(&fpCreateFileW));
        MH_EnableHook(&CreateFileW);
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        MH_DisableHook(&CreateFileW);
        MH_Uninitialize();
    }
    return TRUE;
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
