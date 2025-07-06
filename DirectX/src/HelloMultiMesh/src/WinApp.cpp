#include "pch.h"
#include "WinApp.h"
HWND WinApp::mHwnd = nullptr;

int WinApp::Run(HINSTANCE hInstance, MultiMesh* mesh, int width, int height, const LPCWSTR& winName)
{
    WNDCLASSEX winClass{0};
    winClass.cbSize = sizeof(WNDCLASSEX);
    winClass.style = CS_HREDRAW | CS_VREDRAW;
    winClass.lpfnWndProc = winProc;
    winClass.hInstance = hInstance;
    winClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    winClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    winClass.lpszClassName = winName;

    RegisterClassEx(&winClass);
    RECT rect;
    rect.bottom = height;
    rect.left = 0;
    rect.right = width;
    rect.top = 0;
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
    mHwnd = CreateWindow(winClass.lpszClassName, winName, WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left,
                         rect.bottom - rect.top, nullptr, nullptr, hInstance, mesh);
    mesh->OnInint();
    ShowWindow(mHwnd, SW_SHOW);
    MSG message{nullptr};
    while (message.message != WM_QUIT)
    {
        if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }
    return static_cast<UINT>(message.wParam);
}

HWND WinApp::getWindowHandle()
{
    if (mHwnd == nullptr)
    {
        std::cerr << "window is null" << std::endl;
    }
    return mHwnd;
}

LRESULT WinApp::winProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto mesh = reinterpret_cast<MultiMesh*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (msg)
    {
    case WM_CREATE:
        {
            auto lpCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(lpCreateStruct->lpCreateParams));
        }
        return 0;
    case WM_PAINT:
        mesh->OnUpdate();
        mesh->OnRender();
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
