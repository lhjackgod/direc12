#pragma once

#include <iostream>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <d3dcompiler.h>
#include <Windows.h>
#include <DirectXMath.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;

inline std::string hrToString(HRESULT hr)
{
	char s_str[64];
	sprintf_s(s_str, "ThrowIfFiled code 0x%08X", hr);
	return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
	HrException(HRESULT hr)
		:m_Hr(hr), std::runtime_error(hrToString(hr)){ }
private:
	HRESULT m_Hr;
};

void inline ThrowIfFiled(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw HrException(hr);
	}
}

class Material
{
public:
	Material(int width, int height, std::wstring winName)
		:m_Width(width), m_Height(height), m_WinName(winName) {
		m_ViewPort.Height = height;
		m_ViewPort.MaxDepth = 1.0f;
		m_ViewPort.MinDepth = 0.0f;
		m_ViewPort.TopLeftX = 0.0f;
		m_ViewPort.TopLeftY = 0.0f;
		m_ViewPort.Width = width;

		m_SsiorRect.left = 0.0f;
		m_SsiorRect.right = width;
		m_SsiorRect.bottom = height;
		m_SsiorRect.top = 0.0f;
	}


	void OnInit();
	void OnRender();
	void OnUpdate();
	void OnDestroy();
	void DealWithCommand(int argc, WCHAR* argv[]);
	const WCHAR* getName() { return m_WinName.c_str(); }
	const int getWidth() { return m_Width; }
	const int getHeight() { return m_Height; }

private:
	void LoadPipeline();
	void LoadAssert();
	void waitForFinish();
	void populateCommandList();
	void GetHardwareAdapter(IDXGIAdapter1** adapter, IDXGIFactory7* factory, bool requestHightPerformanceAdapter);
	std::vector<UINT8> GenerateTextureData();

	int m_Width;
	int m_Height;
	std::wstring m_WinName;
	bool useWarp = false;
	const static int framBufferCount = 2;
	UINT RTVTOTALSIZE;
	int m_CurrentBackFrambufferIndex = 0;
	UINT m_FenceValue;
	UINT m_ImageWidth = 256;
	UINT m_ImageHeight = 256;
	UINT m_TexturePixelSize = 4;

	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 uv;
	};

	struct Constant
	{
		DirectX::XMFLOAT4 offset;
		float padding[60];
	};
	static_assert(sizeof(Constant) % 256 == 0, "Constant must be x256");
	Constant m_Constantdata = {};
	ComPtr<ID3D12Device> m_Device;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	ComPtr<IDXGISwapChain4> m_SwapChain;
	ComPtr<ID3D12Resource> m_Frambuffer[framBufferCount];
	ComPtr<ID3D12PipelineState> m_PiplineSate;
	ComPtr<ID3D12Fence1> m_Fence;
	ComPtr<ID3D12DescriptorHeap> m_rtvDescritorHeap;
	ComPtr<ID3D12RootSignature> m_RootSignature;
	ComPtr<ID3D12Resource> m_VertexBuffer;
	ComPtr<ID3D12Resource> m_gpuTextureResource;
	ComPtr<ID3D12DescriptorHeap> m_SrvDescriptorHeap;
	ComPtr<ID3D12Resource2> m_ConstantBuffer;
	HANDLE m_FenceEvent;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
	D3D12_VIEWPORT m_ViewPort;
	D3D12_RECT m_SsiorRect;
	UINT m_srvSize;
	ComPtr<ID3D12Resource> m_IndexBuuferUploadHeap;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
};

