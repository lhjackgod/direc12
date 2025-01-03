#include <iostream>
#include<wrl.h>
#include <d3d12.h>
#include "d3dx12.h"
#include<dxgi1_6.h>
using Microsoft::WRL::ComPtr;
class HelloTriangle
{
public:
	HelloTriangle()
	{
		LoadPipeline();
	}
private:
	void LoadPipeline();
private:
	HWND m_Window;
	long m_Width = 800;
	long m_Height = 600;
	ComPtr<ID3D12Device> m_Device;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<IDXGISwapChain> m_SwapChain;
	ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;
	static const int swapChainBufferCount = 2;
	UINT m_RTVDescriptorMaxCount;
	ComPtr<ID3D12Resource> m_SwapChainBuffer[swapChainBufferCount];
};