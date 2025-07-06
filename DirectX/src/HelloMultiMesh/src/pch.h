#pragma once
#include <string>
#include <direct.h>
#include "d3dx12.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <iostream>
#include <DirectXMath.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
using Microsoft::WRL::ComPtr;

inline std::string MyToString(const HRESULT& hr)
{
    char buffer[512];
    sprintf_s(buffer, "HRESULT code 0x%08X", static_cast<UINT>(hr));
    return buffer;
}

class MyException : public std::runtime_error
{
public:
    MyException(HRESULT hr)
        : std::runtime_error(MyToString(hr))
    {
    }
};

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw MyException(hr);
    }
}
