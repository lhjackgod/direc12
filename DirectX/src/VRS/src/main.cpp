#include "WinApp.h"

int WINAPI main(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow)
{
	try
	{
		HelloTriangle cube(800, 600, L"Cube");
		WinApp::Run(hInstance, &cube, L"Cube");
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}