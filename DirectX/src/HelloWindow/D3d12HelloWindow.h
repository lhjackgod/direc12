#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <iostream>
#include <string>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
using Microsoft::WRL::ComPtr;

inline std::wstring AnsiToWString(const std::string & str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

class DxException : public std::runtime_error
{
public:
	DxException(HRESULT hr__, const std::wstring& functionName, const std::wstring& filename, int lineNumber)
		: std::runtime_error("error") {
		filename + functionName + std::to_wstring(hr__);
	}
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif
class HelloWindow {
public:
	HelloWindow(UINT width, UINT height, std::wstring name);
	
	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();
	virtual int GetWidth() { return m_ClientWidth; }
	virtual int GetHeight() { return m_ClientHeight; }

private:
	static const UINT FramCount = 2;
	int m_ClientWidth = 800;
	int m_ClientHeight = 600;
	DXGI_FORMAT m_BackFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	UINT m_FrambufferIndex;
	UINT m_rtvDescriptorSize;
	UINT m_FenceValue;
	HANDLE m_FenceEvent;

	ComPtr<IDXGIFactory4> m_Factory;
	ComPtr<ID3D12Device> m_Device;
	ComPtr<IDXGISwapChain3> m_Swapchain;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	ComPtr<ID3D12DescriptorHeap> m_RtvDescriptorHeap;
	ComPtr<ID3D12Resource> m_RenderTargets[FramCount];
	ComPtr<ID3D12Fence> m_Fence;
	ComPtr<ID3D12PipelineState> m_PipelineState;

	HINSTANCE m_Hstance;

	void LoadPipeline();
	void LoadAsserts();
	void PopulateCommandList();
	void WaitForPreviousFrame();
};