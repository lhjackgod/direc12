#pragma once


#include "d3dx12.h"
#include "MultiMesh.h"

class WinApp
{
public:
    static int Run(HINSTANCE hInstance, MultiMesh* mesh, int width, int height, const LPCWSTR& winName);
    static HWND getWindowHandle();
    static LRESULT winProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    static HWND mHwnd;
};
