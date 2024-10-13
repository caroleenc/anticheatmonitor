#include <windows.h>
#include <iostream>
#include <tlhelp32.h>

DWORD GetProcessId(const wchar_t* processName) {
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &processEntry)) {
        do {
            if (wcscmp(processEntry.szExeFile, processName) == 0) {
                CloseHandle(snapshot);
                pid = processEntry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &processEntry));
    }
    CloseHandle(snapshot);
    return pid;
}

bool InjectDLL(DWORD pid, const char* dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == NULL) {
        DWORD error = GetLastError();
        std::cerr << "Failed to open target process. Error: " << error << std::endl;
        return false;
    }

    void* pAlloc = VirtualAllocEx(hProcess, nullptr, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pAlloc == nullptr) {
        std::cerr << "Failed to allocate memory in the target process." << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    WriteProcessMemory(hProcess, pAlloc, dllPath, strlen(dllPath) + 1, nullptr);

    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0,
        (LPTHREAD_START_ROUTINE)LoadLibraryA, pAlloc, 0, nullptr);
    if (hThread == NULL) {
        std::cerr << "Failed to create remote thread." << std::endl;
        VirtualFreeEx(hProcess, pAlloc, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);
    VirtualFreeEx(hProcess, pAlloc, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    return true;
}

int main() {
    const wchar_t* targetProcess = L"BEService.exe";
    // dll loc
    const char* dllPath = "C:\\Users\\Caroleen\\source\\repos\\MyHookDLL\\MyHookDLL\\x64\\Debug\\MyHookDLL.dll";

    DWORD pid = GetProcessId(targetProcess);
    if (pid == 0) {
        std::cerr << "Failed to find target process." << std::endl;
        return 1;
    }

    if (InjectDLL(pid, dllPath)) {
        std::cout << "DLL successfully injected." << std::endl;
    }
    else {
        std::cerr << "DLL injection failed." << std::endl;
    }
    // keep the console open until ky pressed
    std::cout << "Press any key to exit..." << std::endl;
    std::cin.get();  // wait for user to press Enter

    return 0;
}
