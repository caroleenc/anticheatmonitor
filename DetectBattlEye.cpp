#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>

HWND hEdit;

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
				return true;
			}
		} while (Process32Next(snapshot, &entry));
	}
	CloseHandle(snapshot);
	return false;
}

// Function to append text to the log
void AppendTextToLog(HWND hEdit, const TCHAR* newText) {
	int len = GetWindowTextLength(hEdit);
	SendMessage(hEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
	SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)newText);
}

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow
	) {

	// create a main windwo
	HWND hwnd = CreateWindowEx(
		0, 
		WC_DIALOG, 
		_T("Anti-Cheat Monitor"), 
		WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT, 
		500, 400,
		NULL, NULL, hInstance, NULL);

	// create a console log area
	hEdit = CreateWindowEx(
		0,
		_T("EDIT"),
		_T(""),
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
		10, 10, 480, 350,
		hwnd, NULL, hInstance, NULL);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	const TCHAR* targetProcess = _T("BEService.exe");

	// if processName is running, display "Hello World"
	if (IsProcessRunning(targetProcess)) {
		AppendTextToLog(hEdit, _T("Hello World! BattlEye Anti-Cheat Service Detected!\r\n"));
		MessageBox(NULL, _T("Hello World!"), _T("Process Detected"), MB_OK);
	} else {
		AppendTextToLog(hEdit, _T("Program Not Running"));
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return (int)msg.wParam;
}