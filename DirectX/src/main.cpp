#include "Material/Material.h"
#include "Material/MaterialWinApplication.h"
int WINAPI main(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow)
{
	try
	{
		Material materila(800, 600, L"jack");
		MaterialWinApplication::Run(hInstance, &materila);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	
}