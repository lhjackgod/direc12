#pragma once
#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <iostream>
#include <wrl.h>
#include <dxgi1_6.h>
#include "d3dx12.h"
#include <d3d12.h>
#include <d3dcompiler.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

using Microsoft::WRL::ComPtr;

inline std::string getHrCode(HRESULT hr)
{
	char s_str[64];
	sprintf_s(s_str, "HRESULT CODE 0x%08X", static_cast<UINT>(hr));
	return s_str;
}

class MyHRESULTFailed : public std::runtime_error
{
public:
	MyHRESULTFailed(HRESULT hr)
		:m_hr(hr), std::runtime_error(getHrCode(hr))
	{

	}
private:
	HRESULT m_hr;
};

void ThrowIfFailed(HRESULT hr);
class D3DApp
{
protected:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();
	D3DApp(const D3DApp& other) = delete;
	D3DApp& operator = (const D3DApp& other) = delete;

public:
	virtual bool Initialize();
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static D3DApp* getApp() { return mApp; }
	int Run();
protected:
	virtual void OnResize();
	virtual void Update() = 0;
	virtual void Draw() = 0;

protected:
	bool InitMainWindow();
	bool InitDirectx3D();
	
	void LogAdapter();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
	void CreateCommandObjects();
	void createSwapChain();
	void createRtvAndDsvDescriptorHeaps();
	void FlushCommandQueue();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView();
	ID3D12Resource* CurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencileView() const;
	
protected:
	static D3DApp* mApp; //only one
	HINSTANCE m_AppInstance = nullptr;
	HWND m_Window;
	UINT m_RTVDescriptorSize;
	UINT m_DSVDescriptorSize;
	UINT m_CBVSRVUAVSize;
	UINT m4xMsaaQuality;
	int m_CurrentBackBuffer = 0;
	static const int SwapChainBufferCount = 2;

	ComPtr<IDXGIFactory4> m_Factory;
	ComPtr<ID3D12Device> m_Device;
	ComPtr<ID3D12Fence> m_Fence;
	UINT m_CurrentFence;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	ComPtr<IDXGISwapChain> m_SwapChain;
	ComPtr<ID3D12DescriptorHeap> m_RtvDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> m_DsvDescriptorHeap;
	ComPtr<ID3D12Resource> m_SwapchainBuffers[SwapChainBufferCount];
	ComPtr<ID3D12Resource> m_DepthStencileBuffer;
	CD3DX12_VIEWPORT m_ScreenViewport;
	D3D12_RECT m_ScissorRect;

	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	int m_ClientWidth = 800;
	int m_ClientHeight = 600;
	
};

