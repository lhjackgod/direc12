#include "Win32Application.h"

HWND Win32Application::m_Hwnd;
void Win32Application::Run(HelloWindow* pSample, HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASSEX WndClassEx{ 0 };
	WndClassEx.cbSize = sizeof(WNDCLASSEX);
	WndClassEx.style = CS_HREDRAW | CS_VREDRAW;
	WndClassEx.lpfnWndProc = WindowProc;
	WndClassEx.hInstance = hInstance;
	WndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClassEx.lpszClassName = L"MainWindowClass";
	RegisterClassEx(&WndClassEx);

	RECT windowRect{ 0,0,pSample->GetWidth(), pSample->GetHeight() };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);

	m_Hwnd = CreateWindow(WndClassEx.lpszClassName,
		L"Main",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr, nullptr,
		hInstance, pSample);

	pSample->OnInit();

	
	
}

LRESULT Win32Application::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
