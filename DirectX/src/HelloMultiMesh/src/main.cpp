#include "pch.h"

#include "MultiMesh.h"
#include "WinApp.h"

int WINAPI main(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow)
{
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
