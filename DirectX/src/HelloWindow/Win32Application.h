#ifdef CREATEWINDOW
#pragma once
#include "HelloWindow/D3d12HelloWindow.h"
class Win32Application
{
public:
	static int Run(HelloWindow* pSample, HINSTANCE hInstance, int nCmdShow);
	static HWND GetHwnd() { return m_Hwnd; }
protected:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	static HWND m_Hwnd;
};
#endif
