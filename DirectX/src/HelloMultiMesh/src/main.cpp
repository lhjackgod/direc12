#include "pch.h"

#include "MultiMesh.h"
#include "WinApp.h"

int WINAPI main(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow)
{
    HINSTANCE hUser32 = LoadLibrary(L"user32.dll");
    if (hUser32)
    {
        using SetProcessDPIAware = BOOL(WINAPI*)(void);
        auto pSetProcessDPIAware = (SetProcessDPIAware)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (pSetProcessDPIAware)
        {
            pSetProcessDPIAware();
        }
        FreeLibrary(hUser32);
    }
    try
    {
        MultiMesh mesh(800, 600);
        WinApp::Run(hInstance, &mesh, 800, 600, L"MultiMesh");
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
