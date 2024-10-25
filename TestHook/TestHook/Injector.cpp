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
    // HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
    if (hProcess == NULL) {
        DWORD error = GetLastError();
        std::cerr << "Failed to open target process. Error: " << error << std::endl;
        return false;
    }

    LPVOID pszLibFileRemote = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    // void* pAlloc = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pszLibFileRemote == NULL) {
        std::cerr << "Failed to allocate memory in the target process." << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    WriteProcessMemory(hProcess, pszLibFileRemote, dllPath, strlen(dllPath) + 1, NULL);

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryA, pszLibFileRemote, NULL, NULL);
    if (hThread == NULL) {
        std::cerr << "Failed to create remote thread." << std::endl;
        VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);
    VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    return true;
}

int main() {
    const wchar_t* targetProcess = L"BEService.exe";
    // dll loc
    // const char* dllPath = "C:\\Users\\Caroleen\\source\\repos\\MyHookDLL\\MyHookDLL\\x64\\Debug\\MyHookDLL.dll";
    const char* dllPath = "C:\\Users\\Caroleen\\source\\repos\\vac-hooks\\Debug\\vac-hooks.dll";
    DWORD pid = GetProcessId(targetProcess);

    if (pid == 0) {
        std::cerr << "Failed to find target process." << std::endl;
        return 1;
    }
    else {
        std::cout << "Tagret PID is: " << pid << std::endl;
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
