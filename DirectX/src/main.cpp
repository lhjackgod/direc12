#include "Material/Material.h"
#include "Material/MaterialWinApplication.h"
int WINAPI main(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow)
{
	try
	{
		Material material(800, 600, L"Material");
		MaterialWinApplication::Run(hInstance, &material);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	
}