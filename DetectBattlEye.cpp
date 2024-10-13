#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>
#include "MinHook.h"
#include <ctime>

HWND hEdit, hEditFiles, hEditKeys;
bool detectionRunning = false;
HANDLE detectionThread = NULL;

// hooked functions
typedef HANDLE(WINAPI* CreateFileWType)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef HANDLE(WINAPI* CreateFileAType)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef LSTATUS(WINAPI* RegOpenKeyExWType)(HKEY, LPCWSTR, DWORD, REGSAM, PHKEY);

//hooked function pointers
CreateFileWType TrueCreateFileW = nullptr;
CreateFileAType TrueCreateFileA = nullptr;
RegOpenKeyExWType TrueRegOpenKeyExW = nullptr;

// get timestamp
void GetCurrentTimestamp(wchar_t* buffer, size_t bufferSize) {
	time_t rawTime;
	struct tm timeInfo;

	time(&rawTime);
	localtime_s(&timeInfo, &rawTime);
	wcsftime(buffer, bufferSize, L"%H:%M:S", &timeInfo);
}

// Function to append text to the log
void AppendTextToLog(HWND hEdit, const TCHAR* newText) {
	int len = GetWindowTextLength(hEdit);
	SendMessage(hEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
	SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)newText);
}

HANDLE WINAPI HookCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
	DWORD threadId = GetCurrentThreadId();
	/*wchar_t timeBuffer[80];
	GetCurrentTimestamp(timeBuffer, sizeof(timeBuffer) / sizeof(timeBuffer[0]));*/

	TCHAR timestamp[32];
	GetCurrentTimestamp(timestamp, sizeof(timestamp) / sizeof(TCHAR));
	AppendTextToLog(hEditFiles, timestamp);
	AppendTextToLog(hEditFiles, _T("File Accessed: "));
	AppendTextToLog(hEditFiles, lpFileName);
	AppendTextToLog(hEditFiles, _T("\r\n"));

	wchar_t message[256];
	swprintf_s(message, L"[%s] ThreadId: %lu, CreateFileW called on %s\r\n", timestamp, threadId, lpFileName);
	AppendTextToLog(hEdit, message);

	return TrueCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}
HANDLE WINAPI HookCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
	TCHAR timestamp[32];
	GetCurrentTimestamp(timestamp, sizeof(timestamp) / sizeof(TCHAR));
	AppendTextToLog(hEditFiles, timestamp);
	TCHAR fileNameW[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, lpFileName, -1, fileNameW, MAX_PATH);
	AppendTextToLog(hEditFiles, _T("File Accessed: "));
	AppendTextToLog(hEditFiles, fileNameW);
	AppendTextToLog(hEditFiles, _T("\r\n"));
	return TrueCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

LSTATUS WINAPI HookRegOpenKeyExW(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult) {
	TCHAR timestamp[32];
	GetCurrentTimestamp(timestamp, sizeof(timestamp) / sizeof(TCHAR));
	AppendTextToLog(hEditKeys, timestamp);
	AppendTextToLog(hEditKeys, _T("Registry Key Accessed: "));
	AppendTextToLog(hEditKeys, lpSubKey);
	AppendTextToLog(hEditKeys, _T("\r\n"));
	return TrueRegOpenKeyExW(hKey, lpSubKey, ulOptions, samDesired, phkResult);
}

// Functin to check if specific processName running the CreateToolhelp32Snapshot
// and Process32First functions
bool IsProcessRunning(const TCHAR* processName) {
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	
	if (Process32First(snapshot, &entry)) {
		do {
			if (!_tcsicmp(entry.szExeFile, processName)) {
				CloseHandle(snapshot);
				AppendTextToLog(hEdit, _T("Program detected. \r\n"));
				return true;
			}
		} while (Process32Next(snapshot, &entry));
	}
	CloseHandle(snapshot);
	return false;
}


DWORD WINAPI DetectionThreadProc(LPVOID lpParam) {
	const TCHAR* targetProcess = _T("BEService.exe");

	if (MH_Initialize() != MH_OK ||
		MH_CreateHook(&CreateFileW, &HookCreateFileW, reinterpret_cast<LPVOID*>(&TrueCreateFileW)) != MH_OK ||
		MH_CreateHook(&CreateFileA, &HookCreateFileA, reinterpret_cast<LPVOID*>(&TrueCreateFileA)) != MH_OK ||
		MH_CreateHook(&RegOpenKeyExW, &HookRegOpenKeyExW, reinterpret_cast<LPVOID*>(&TrueRegOpenKeyExW)) != MH_OK) {
		AppendTextToLog(hEdit, _T("Failed to initialize MinHook. \r\n"));
		return 0;
	}

	MH_EnableHook(&CreateFileW);
	MH_EnableHook(&CreateFileA);
	MH_EnableHook(&RegOpenKeyExW);

	while (detectionRunning) {
			if (!IsProcessRunning(targetProcess)) {
				AppendTextToLog(hEdit, _T("Program not detected. \r\n"));
			}
		Sleep(5000);
	}

	//Hook Functions
	MH_DisableHook(&CreateFileW);
	MH_DisableHook(&RegOpenKeyExW);
	MH_Uninitialize();
	return 0;
}

// handle button clicks
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_COMMAND:
			//start
			if (LOWORD(wParam) == 1) {
				if (!detectionRunning) {
					detectionRunning = true;
					detectionThread = CreateThread(NULL, 0, DetectionThreadProc, NULL, 0, NULL);
					AppendTextToLog(hEdit, _T("Starting detection...\r\n"));
				}
			}
			else if (LOWORD(wParam) == 2) {
				detectionRunning = false;
				if (detectionThread != NULL) {
					WaitForSingleObject(detectionThread, INFINITE);
					CloseHandle(detectionThread);
					detectionThread = NULL;
					AppendTextToLog(hEdit, _T("Stopping detection... \r\n"));
				}
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow
	) {

	//register the window class
	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = _T("AntiCheatMonitor");
	RegisterClass(&wc);

	// create a main windwo
	HWND hwnd = CreateWindowEx(0, _T("AntiCheatMonitor"), _T("Anti-Cheat Monitor"),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 700, 900,
		NULL, NULL, hInstance, NULL);

	// create a console log area
	hEdit = CreateWindowEx(0, _T("EDIT"), _T("Console Logging: \r\n"),
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
		10, 40, 690, 350,
		hwnd, NULL, hInstance, NULL);

	// create an area for files
	hEditFiles = CreateWindowEx(0, _T("EDIT"), _T("Files:\r\n"),
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
		10, 370, 340, 400,
		hwnd, NULL, hInstance, NULL);

	//// create an area for registry keys
	hEditKeys = CreateWindowEx(0, _T("EDIT"), _T("Registry Keys: \r\n"),
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
		360, 370, 340, 400,
		hwnd, NULL, hInstance, NULL);

	//create the start btn
	CreateWindowEx(0, _T("BUTTON"), _T("Start"),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		10, 10, 100, 30,
		hwnd, (HMENU)1, hInstance, NULL);

	//create the stop btn
	CreateWindowEx(0, _T("BUTTON"), _T("Stop"),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		120, 10, 100, 30,
		hwnd, (HMENU)2, hInstance, NULL);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	//// if processName is running, display "Hello World"
	//if (IsProcessRunning(targetProcess)) {
	//	AppendTextToLog(hEdit, _T("Hello World! BattlEye Anti-Cheat Service Detected!\r\n"));
	//	MessageBox(NULL, _T("Hello World!"), _T("Process Detected"), MB_OK);
	//} else {
	//	AppendTextToLog(hEdit, _T("Program Not Running"));
	//}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return (int)msg.wParam;
}
