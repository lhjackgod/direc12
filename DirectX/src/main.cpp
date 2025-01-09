#include "HelloTriangle/HelloTriangleWin.h"
#include "HelloTriangle/HelloTriangle.h"
int WINAPI main(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow)
{
	try
	{
		HelloTriangle helloTriangle(800, 600, L"Triangle");
		HelloTriangleWin::Run(&helloTriangle, hInstance);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	
}