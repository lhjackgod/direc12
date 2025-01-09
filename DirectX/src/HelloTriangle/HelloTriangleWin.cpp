#include "HelloTriangleWin.h"

HWND HelloTriangleWin::m_Hwnd = nullptr;

int HelloTriangleWin::Run(HelloTriangle* pSample, HINSTANCE hInstance)
{
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	pSample->DealWithCommandLineArgs(argc, argv);
	LocalFree(argv); //free the argv

	WNDCLASSEX windowClass{ 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = L"MainWindow";
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.lpfnWndProc = windowProc;
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, static_cast<LONG>(pSample->getWidth()), static_cast<LONG>(pSample->getHeight()) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	m_Hwnd = CreateWindow(windowClass.lpszClassName, pSample->getLpClassName(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top, nullptr, nullptr, hInstance, pSample);
	
	pSample->OnInit();

	ShowWindow(m_Hwnd, SW_SHOW);
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

LRESULT HelloTriangleWin::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HelloTriangle* sample = reinterpret_cast<HelloTriangle*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	switch (msg)
	{
	case WM_CREATE:
	{
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
	}
		return 0;
	case WM_PAINT:
		if (sample)
		{
			sample->OnUpdate();
			sample->OnRender();
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
