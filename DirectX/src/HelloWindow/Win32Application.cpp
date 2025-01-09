#include "Win32Application.h"

HWND Win32Application::m_Hwnd = nullptr;
int Win32Application::Run(HelloWindow* pSample, HINSTANCE hInstance, int nCmdShow)
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

	ShowWindow(m_Hwnd, SW_SHOW);
	std::cout << SW_SHOW << std::endl;
	MSG msg{};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	pSample->OnDestroy();
	return static_cast<char>(msg.wParam);
}

LRESULT Win32Application::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HelloWindow* window = reinterpret_cast<HelloWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	switch (msg)
	{
	case WM_CREATE:
	{
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
	}
		
		return 0;
	case WM_PAINT:
		if (window)
		{
			window->OnUpdate();
			window->OnRender();
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
