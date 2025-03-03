#pragma once
#include <direct.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <iostream>
#include <DirectXMath.h>
#include "Camera.h"
#include <chrono>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;
extern class ConstBufferClass;
struct MathHeap
{
	static DirectX::XMFLOAT4X4 getMatrixIdentity()
	{
		DirectX::XMFLOAT4X4 proj;
		DirectX::XMStoreFloat4x4(&proj, DirectX::XMMatrixIdentity());
		return proj;
	}
};

struct MyColor
{
	static DirectX::XMFLOAT4 White;
	static DirectX::XMFLOAT4 Black;
	static DirectX::XMFLOAT4 Red;
	static DirectX::XMFLOAT4 Green;
	static DirectX::XMFLOAT4 Blue;
	static DirectX::XMFLOAT4 Yellow;
	static DirectX::XMFLOAT4 Cyan;
	static DirectX::XMFLOAT4 Magenta;
};

inline std::string HrToString(HRESULT hr)
	{
		char s_str[64] = {};
		sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
		return std::string(s_str);
	}

	class HrException : public std::runtime_error
	{
	public:
		HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_Hr(hr) {}
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
		HelloTriangle(UINT width, UINT height, const std::wstring& name) :
			m_Width(width), m_Height(height), lPclassName(name) {
			m_AspectRatio = (float)width / (float)height;
		}

		UINT getWidth() { return m_Width; }
		UINT getHeight() { return m_Height; }
		const WCHAR* getLpClassName() { return lPclassName.c_str(); }

		void OnInit();
		void OnDestroy();
		void OnRender();
		void OnUpdate();
		void DealWithCommandLineArgs(int argc, WCHAR* argv[]);
		void OnMouseOn(WPARAM wParam, int xPos, int yPos);
		void OnMouseUp(WPARAM wParam, int xPos, int yPos);
		void OnMouseMove(WPARAM wParam, int xPos, int yPos);
	private:

		bool m_UseWarpDevice = false;
		static const int FramBufferCount = 2;
		UINT m_Width;
		UINT m_Height;
		UINT m_FramIndex;
		UINT m_RtvDescriptorSize;
		UINT m_DsvDescriptorSize;
		UINT m_CbvSrvUavDescriptorSize;
		UINT m_FenceValue;
		std::wstring lPclassName;
		float m_AspectRatio;

		struct Vertex
		{
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT4 color;
		};
		struct ConstBuffer
		{
			DirectX::XMFLOAT4 durationTime;
			DirectX::XMFLOAT4X4 mvp;
		}m_MVPMatrix;
		struct ConstBuffer2
		{
			float time;
		}Time;
		ComPtr<ID3D12Device> m_Device;
		ComPtr<ID3D12CommandQueue> m_CommandQueue;
		ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
		ComPtr<ID3D12GraphicsCommandList> m_CommandList;
		ComPtr<IDXGISwapChain3> m_SwapChain;
		ComPtr<ID3D12DescriptorHeap> m_RtvDescriptorHeap;
		ComPtr<ID3D12DescriptorHeap> m_DsvDescriptorHeap;
		ComPtr<ID3D12Resource> m_RenderBuffer[FramBufferCount];
		ComPtr<ID3D12Resource1> m_DsvBuffer;
		ComPtr<ID3D12Fence1> m_Fence;
		ComPtr<ID3D12PipelineState> m_PipelineState;
		ComPtr<ID3D12RootSignature> m_RootSignature;
		ComPtr<ID3D12Resource> m_VertexBuffer;
		ComPtr<ID3D12Resource> m_IndexBuffer;
		ComPtr<ID3D12Resource> m_UploadBuffer;
		ComPtr<ID3D12DescriptorHeap> m_ConstBufferViewHeap;
		ComPtr<ID3D12Resource> m_ConstBuffer;
		ComPtr<ID3D12Resource> m_ConstBufferUpload;
		UINT m_VertexBufferSize;
		UINT m_IndexBufferSize;
		ComPtr<ID3DBlob> m_VertexShader;
		ComPtr<ID3DBlob> m_PixelShader;
		D3D12_VIEWPORT m_ViewPort;
		D3D12_RECT m_Scissor;
		HANDLE m_FenceEvent;
		Camera m_MainCamera;
		DirectX::XMMATRIX m_CubeModel = DirectX::XMMatrixIdentity();
		DirectX::XMVECTOR cubePos;
		int currentMouseLoX;
		int currentMouseLoY;
		float theta = 0.0f;
		float phi = 0.0f;
		std::chrono::high_resolution_clock::time_point startTime;
		std::chrono::high_resolution_clock::time_point endTime;
	private:
		void LoadPipeline();
		void LoadAsserts();
		void waitForFinish();
		void populateCommandList();
	    void createDevice(ComPtr<IDXGIFactory4>& factory);
		void createCommandQueue();
		void createCommandAllocate();
		void createSwapChain(ComPtr<IDXGIFactory4>& factory);
		void createDescriptorHeap();
		void createRTV();
		void createDSV();
		void createFence();
		void createRootSignature();
		void createShader();
		void createVertexBufferAndInputLayout();
		void createIndexBuffer();
		void createDefaultBuffer(const void* data, const UINT byteSize, ComPtr<ID3D12Resource>& defaultBuffer,
			ComPtr<ID3D12Resource>& upLoad);
		void createPipelineState();
		D3D12_VERTEX_BUFFER_VIEW getVertexBufferView();
		D3D12_INDEX_BUFFER_VIEW getIndexBufferView();
		void createConstBuffer();
		void createConstBufferView();
		void UpdateBuffer(ComPtr<ID3D12Resource>& defaultBuffer, ComPtr<ID3D12Resource>& uploadBuffer, UINT byteSize,
			void* data);
		float getDurationTime();
	};


inline UINT CalcConstBufferByteSize(UINT byteSize)
{
	return (byteSize + 255) & ~255;
}