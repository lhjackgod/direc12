#pragma once
#include "HelloBandles/Bandles.h"
class BandlesApp
{
public:
	static char Run(Bandles* m_Bandles, HINSTANCE hInstance);

	static HWND getInstance() { return m_Hwnd; }

private:
	static HWND m_Hwnd;

	static LRESULT windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

