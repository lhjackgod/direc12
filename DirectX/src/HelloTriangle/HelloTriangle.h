#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <iostream>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

inline std::string HrToString(HRESULT hr)
{
	char s_str[64] = {};
	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
	return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
	HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_Hr(hr){}
private:
	HRESULT m_Hr;
};

inline void ThrowIfFiled(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw HrException(hr);
	}
}

using Microsoft::WRL::ComPtr;
class HelloTriangle
{
public:
	HelloTriangle(UINT width, UINT height, const std::wstring& name):
		m_Width(width), m_Height(height), lPclassName(name){ }

	UINT getWidth() { return m_Width; }
	UINT getHeight() { return m_Height; }
	const WCHAR* getLpClassName() { return lPclassName.c_str(); }

	void OnInit();
	void OnDestroy();
	void OnRender();
	void OnUpdate();
	void DealWithCommandLineArgs(int argc, WCHAR* argv[]);
private:

	bool m_UseWarpDevice = false;
	static const int FramBufferCount = 2;
	UINT m_Width;
	UINT m_Height;
	UINT m_FramIndex;
	UINT m_RtvDescriptorSize;
	UINT m_FenceValue;
	std::wstring lPclassName;

	ComPtr<ID3D12Device> m_Device;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	ComPtr<IDXGISwapChain3> m_SwapChain;
	ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;
	ComPtr<ID3D12Resource> m_RenderBuffer[FramBufferCount];
	ComPtr<ID3D12Fence1> m_Fence;
	ComPtr<ID3D12PipelineState> m_PipelineState;
	ComPtr<ID3D12RootSignature> m_RootSignature;
	HANDLE m_FenceEvent;
	void LoadPipeline();
	void LoadAsserts();
	void waitForFinish();
	void populateCommandList();
};

