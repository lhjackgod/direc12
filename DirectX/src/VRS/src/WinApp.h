#pragma once
#include "CubeClass.h"
#include <iostream>
class WinApp
{
public:
	static int Run(HINSTANCE hInstance, HelloTriangle* cube, const LPCWCH& winName);
	static HWND getHwnd() { if (m_Hwnd == nullptr) std::cout << "jack" << std::endl; return m_Hwnd; }
	static LRESULT windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	static HWND m_Hwnd;
};

