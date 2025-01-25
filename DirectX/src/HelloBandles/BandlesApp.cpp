#include "BandlesApp.h"

HWND BandlesApp::m_Hwnd = nullptr;

char BandlesApp::Run(Bandles* m_Bandles, HINSTANCE hInstance)
{
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);
	

	WNDCLASSEX wndClassEx{};
	wndClassEx.hInstance = hInstance;
	wndClassEx.lpszClassName = L"BandlesApp";
	wndClassEx.cbSize = sizeof(WNDCLASSEX);
	wndClassEx.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClassEx.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wndClassEx.lpfnWndProc = windowProc;
	wndClassEx.style = CS_VREDRAW | CS_HREDRAW;
	RegisterClassEx(&wndClassEx);

	RECT windowSize = {0, 0, (LONG)(m_Bandles->getWidth()), (LONG)(m_Bandles->getHeight())};
	AdjustWindowRect(&windowSize, WS_OVERLAPPEDWINDOW, false);

	m_Hwnd = CreateWindow(wndClassEx.lpszClassName, m_Bandles->getName(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
	windowSize.right - windowSize.left, windowSize.bottom - windowSize.top,
	nullptr, nullptr, hInstance, m_Bandles);

	m_Bandles->onInit();
	ShowWindow(m_Hwnd, SW_SHOW);

	MSG msg{0};

	while(msg.message != WM_QUIT)
	{
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	m_Bandles->onDesctroy();
	return static_cast<char>(msg.wParam);
}

LRESULT BandlesApp::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Bandles* pBandles = reinterpret_cast<Bandles*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	switch(msg)
	{
		case WM_CREATE:
			{
				LPCREATESTRUCT lPCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
				SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(lPCreateStruct->lpCreateParams));
			}
			return 0;
		case WM_PAINT:
			pBandles->onUpdate();
			pBandles->onRender();
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}