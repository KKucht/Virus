#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>


#pragma comment(lib, "WSOCK32.lib")
HANDLE pipe;
HWND mainFrame1;
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

LRESULT CALLBACK WindowProc1(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
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
	SOCKET si;
};


char numberAccount1[27] = "00000000000000000000000000";
char numberAccount2[27] = "00000000000000000000000000";
DWORD WINAPI servicing(void* arg) {
	struct DataOne* data = (struct DataOne*)arg;
	SOCKET si = data->si;
	char buf[512] = "i'm ready.";
	//MessageBoxA(mainFrame1, "jest polaczenie.", "jest polaczenie.", 0);
	send(si, buf, 512, 0);
	while (recv(si, buf, 512, 0) > 0)
	{
		if (checkIfItsNumber(buf)) {
			//MessageBoxA(mainFrame1, "to liczba.", "jest polaczenie.", 0);
			strncpy(numberAccount2, buf, 27);
		}
		if (strcmp(numberAccount1, numberAccount2) != 0) {
			//MessageBoxA(mainFrame1, "nowa liczba.", "jest polaczenie.", 0);
			strncpy(numberAccount1, numberAccount2, 27);
			WriteFile(pipe, numberAccount1, 27, NULL, NULL);
		}
		Sleep(3000);
		send(si, buf, 512, 0);
		//MessageBoxA(mainFrame1, buf, "jest polaczenie.", 0);
	}
	return 0;
}

const char* cmd = "C:/Users/Krzys/Desktop/pr5i6/wirus_klient2/Debug/wirus_klient";


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{

	const wchar_t CLASS_NAME[] = L"MyClientConnectClass";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc1;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	RegisterClass(&wc);

	mainFrame1 = CreateWindowEx(0, CLASS_NAME, L"Client_connect", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 250, 250, NULL, NULL, hInstance, NULL);
		

	SOCKET s;
	struct sockaddr_in sa;
	WSADATA wsas;
	WORD wersja;
	wersja = MAKEWORD(2, 0);
	WSAStartup(wersja, &wsas);
	s = socket(AF_INET, SOCK_STREAM, 0);
	memset((void*)(&sa), 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(55555);
	sa.sin_addr.s_addr = inet_addr("127.0.0.1");

	int result;
	result = connect(s, (struct sockaddr FAR*) & sa, sizeof(sa));
	if (result == SOCKET_ERROR)
	{
		MessageBoxA(mainFrame1, "We don't have connect.", "We don't have connect.", 0);
		return -1;
	}
	pipe = CreateNamedPipeA("\\\\.\\pipe\\xd", PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE,1,0,0, 10000,0);
	if (pipe == INVALID_HANDLE_VALUE) {
		MessageBoxA(mainFrame1, "Could not make pipe - connect. ", "Could not make pipe.", 0);
		return -1;
	}
	DataOne x;
	x.si = s;
	DWORD id;
	HANDLE thread = CreateThread(NULL, 0, servicing, (void*)&x, 0, &id);
	if (thread != INVALID_HANDLE_VALUE)
	{
		SetThreadPriority(thread, THREAD_PRIORITY_NORMAL);
	}
	PROCESS_INFORMATION pi;
	STARTUPINFOA si;
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	if (!CreateProcessA(NULL,
		(LPSTR)cmd,
		NULL,
		NULL,
		CREATE_NEW_CONSOLE,
		0,
		NULL,
		NULL,
		&si,
		&pi))
	{
		MessageBoxA(mainFrame1, "Could not create process.", "Could not create process.", 0);
		return -1;
	}
	ConnectNamedPipe(pipe, NULL);
	MSG msg;
	while (GetMessage(&msg, (HWND)NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	CloseHandle(pipe);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	closesocket(s);
	WSACleanup();
	return msg.wParam;
}