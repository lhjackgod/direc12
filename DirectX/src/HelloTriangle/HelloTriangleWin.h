#ifdef HelloTriangle
#pragma once

#include "HelloTriangle/HelloTriangle.h"
namespace HelloTriangle
{
	class HelloTriangleWin
	{
	public:
		static int Run(HelloTriangle* pSample, HINSTANCE hInstace);

		static HWND getWnd() { return m_Hwnd; }
		static LRESULT windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	private:
		static HWND m_Hwnd;
	};
}

#endif
