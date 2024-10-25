// dllmain.cpp : Defines the entry point for the DLL application.
/* #include "pch.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
*/
#include "pch.h"
#include <windows.h>
#include <iostream>
#include "MinHook.h"

// define a function pointer type for CreateFileW
typedef HANDLE(WINAPI* CreateFileW_t)(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    );

// declare a global function pointer for the original CreateFileW
CreateFileW_t fpCreateFileW = nullptr;

// Hooked CreateFileW function
HANDLE WINAPI HookedCreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
) {
    // log the file name being accessed
    std::wcout << L"CreateFileW called with file: " << lpFileName << std::endl;

    // call the original CreateFileW function
    return fpCreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
        lpSecurityAttributes, dwCreationDisposition,
        dwFlagsAndAttributes, hTemplateFile);
}

// DllMain function: entry point for the DLL
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        // initialize MinHook
        if (MH_Initialize() != MH_OK) {
            return FALSE;
        }

        // create a hook for CreateFileW
        if (MH_CreateHook(&CreateFileW, &HookedCreateFileW,
            reinterpret_cast<LPVOID*>(&fpCreateFileW)) != MH_OK) {
            return FALSE;
        }

        //enable the hook for CreateFileW
        if (MH_EnableHook(&CreateFileW) != MH_OK) {
            return FALSE;
        }
        break;

    case DLL_PROCESS_DETACH:
        //disable the hook and uninitialize MinHook on DLL unload
        MH_DisableHook(&CreateFileW);
        MH_Uninitialize();
        break;
    }
    return TRUE;
}
