#include "HelloBandles/Bandles.h"
#include "HelloBandles/BandlesApp.h"
int WINAPI main(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow)
{
	try
	{
		Bandles m_bandles(800, 600, L"Hello Bandles");
		BandlesApp::Run(&m_bandles, hInstance);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	
}