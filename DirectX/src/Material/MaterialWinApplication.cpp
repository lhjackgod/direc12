#include "MaterialWinApplication.h"

HWND MaterialWinApplication::m_Hwnd = nullptr;

int MaterialWinApplication::Run(HINSTANCE hInstance, Material* material)
{
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	material->DealWithCommand(argc, argv);

	WNDCLASSEX wndCLassEx{0};
	wndCLassEx.cbSize = sizeof(WNDCLASSEX);
	wndCLassEx.hInstance = hInstance;
	wndCLassEx.lpszClassName = L"WinApplication";
	wndCLassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndCLassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndCLassEx.lpfnWndProc = WinProc;
	wndCLassEx.style = CS_VREDRAW | CS_HREDRAW;
	RegisterClassEx(&wndCLassEx);

	RECT borderMsg{ 0, 0, material->getWidth(), material->getHeight() };
	AdjustWindowRect(&borderMsg, WS_OVERLAPPEDWINDOW, FALSE);
	m_Hwnd = CreateWindow(wndCLassEx.lpszClassName,
		material->getName(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		borderMsg.right - borderMsg.left, borderMsg.bottom - borderMsg.top, nullptr, nullptr, hInstance, material);

	material->OnInit();
	ShowWindow(m_Hwnd, SW_SHOW);

	MSG msg{ 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	material->OnDestroy();
	return static_cast<char>(msg.wParam);
}

LRESULT MaterialWinApplication::WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Material* pSampler = reinterpret_cast<Material*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	switch (msg)
	{
	case WM_CREATE:
	{
		LPCREATESTRUCT lpCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(lpCreateStruct->lpCreateParams));
	}
		return 0;
	case WM_PAINT:
		pSampler->OnUpdate();
		pSampler->OnRender();		
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
