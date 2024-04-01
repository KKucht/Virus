#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>

#pragma comment(lib, "WSOCK32.lib")



LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect;
	switch (message)
	{
	case WM_CREATE:
	
		return (0);
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
}

struct DataOne
{
	HWND mainFrame;
	HWND newAccountNumber;
};

struct DataTwo
{
	SOCKET si;
	HWND mainFrame;
	HWND newAccountNumber;
};

DWORD WINAPI servicing(void* arg) {
	struct DataTwo* data = (struct DataTwo*)arg;
	SOCKET si = data->si;
	char buf[512];
	//MessageBoxA(mainFrame, "watek2 - connect. ", "Could not make pipe.", 0);
	while (recv(si, buf, 512, 0) > 0)
	{
		GetWindowTextA(data->newAccountNumber, buf, GetWindowTextLengthA(data->newAccountNumber) + 1);
		send(si, buf, 512, 0);
		Sleep(200);
	}
	return 0;
}

DWORD WINAPI connecting(void* arg) {
	struct DataOne* data = (struct DataOne*)arg;
	WSADATA wsas;
	int result;
	WORD wersja;
	wersja = MAKEWORD(1, 1);
	result = WSAStartup(wersja, &wsas);
	SOCKET s;
	s = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sa;
	memset((void*)(&sa), 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(55555);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	result = bind(s, (struct sockaddr FAR*) & sa, sizeof(sa));
	result = listen(s, 5);
	SOCKET si;
	struct sockaddr_in sc;
	int lenc;
	for (;;)
	{
		lenc = sizeof(sc);
		si = accept(s, (struct sockaddr FAR*) & sc, &lenc);
		DataTwo x;
		x.mainFrame = data->mainFrame;
		x.newAccountNumber = data->newAccountNumber;
		x.si = si;
		DWORD id;
		HANDLE thread = CreateThread(NULL, 0, servicing, (void*)&x, 0, &id);
		
		if (thread != INVALID_HANDLE_VALUE)
		{
			SetThreadPriority(thread, THREAD_PRIORITY_NORMAL);
			Sleep(5000);
		}
		
	}
	return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) {
	const wchar_t CLASS_NAME[] = L"MyServerClass";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	RegisterClass(&wc);

	HWND mainFrame = CreateWindowEx(0, CLASS_NAME, L"Server", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 250, 250, NULL, NULL, hInstance, NULL);
	HWND newAccountNumber = CreateWindowEx(WS_EX_WINDOWEDGE, TEXT("Edit"), TEXT("00000000000000000000000000"),WS_CHILD | WS_VISIBLE, 5, 5, 220, 20, mainFrame, NULL, NULL, NULL);

	DataOne x;
	x.mainFrame = mainFrame;
	x.newAccountNumber = newAccountNumber;

	DWORD id;
	HANDLE thread = CreateThread(NULL,0, connecting,(void*)&x,0,&id);
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
	return msg.wParam;
}
