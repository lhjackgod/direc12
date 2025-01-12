#pragma once

#include "Material/Material.h"
class MaterialWinApplication
{
public:
	static int Run(HINSTANCE hInstance, Material* material);

	static HWND getWin() { return m_Hwnd; }
private:
	static HWND m_Hwnd;

	static LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

