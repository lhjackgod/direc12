#pragma once
#include <wrl.h>
#include <iostream>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <Windows.h>
#include <DirectXMath.h>
#include <chrono>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
using Microsoft::WRL::ComPtr;

inline std::string hrToString(HRESULT hr)
{
	char s_str[64] = {};
	sprintf_s(s_str, "throwIfFiled code 0x%08X", hr);
	return std::string(s_str);
}

class MyBandlesException : public std::runtime_error
{
public:
	MyBandlesException(HRESULT hr)
		: m_Hresult(hr), std::runtime_error(hrToString(hr))
	{

	}
private:
	HRESULT m_Hresult;
};

inline void ThrowIfFiled(HRESULT hr)
{
	if(FAILED(hr))
	{
		throw MyBandlesException(hr);
	}
}

class Bandles
{
public:
	Bandles(int width, int height, const WCHAR* winName);
	Bandles() = delete;
	Bandles(const Bandles&) = delete;
	int getWidth();
	int getHeight();
	const WCHAR* getName();

	void onInit();
	void onRender();
	void onUpdate();
	void onDesctroy();
	void dealWithCommandLiine(int argc, LPWSTR argv[]);

private:
	int m_Width, m_Height;
	const WCHAR* m_WindowName;
	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_Sissor;
	bool m_UseWarp = false;
	const static int m_FrameCount = 2;
	int m_BackCurrentFrame = 0;
	std::chrono::steady_clock::time_point begin;
	std::chrono::steady_clock::time_point end;
	ComPtr<IDXGIAdapter1> m_Adapter;
	ComPtr<ID3D12Device> m_Device;
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	ComPtr<IDXGISwapChain4> m_SwapChain;
	ComPtr<ID3D12Resource2> m_RtvBuffer[m_FrameCount];
	ComPtr<ID3D12DescriptorHeap> m_RtvDescriptorHeap;
	ComPtr<ID3D12RootSignature> m_RootSignature;
	ComPtr<ID3D12Fence> m_Fence;
	ComPtr<ID3D12PipelineState> m_PipelineState;
	UINT m_RtvDescriptorSize;
	UINT m_FenceValue = 0;
	HANDLE m_FenceEvent;
	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
	ComPtr<ID3D12DescriptorHeap> m_SrvDescriptorHeap;
	UINT m_SrvDescriptorSize;
	ComPtr<ID3D12CommandAllocator> m_BundleAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_Bundle;

	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 color;
	};
private:
	void LoadAssert();
	void LoadPipeline();
	void GetAdapter(ComPtr<IDXGIFactory7>& pFactory, IDXGIAdapter1** adapter);
	void waitForFinish();
	void fillOutCommandList();
};

