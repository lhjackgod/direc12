#include "HelloWindow/D3d12HelloWindow.h"
#include "HelloWindow/Win32Application.h"
int WINAPI main(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow)
{
	HelloWindow mHelloWindow((UINT)800, (UINT)600, L"mainWindow");
	Win32Application::Run(&mHelloWindow, hInstance, cmdShow);
}