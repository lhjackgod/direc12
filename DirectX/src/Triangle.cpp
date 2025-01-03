#include "Triangle.h"
#include <d3dcompiler.h>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

LRESULT WindowProcedure(HWND hwnd, UINT messageType, WPARAM wParam, LPARAM lParam)
{
	if (messageType == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, messageType, wParam, lParam);
}

#ifdef _DEBUG

inline std::string HrToString(HRESULT hr)
{
	char s_str[64];
	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
	return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
	HrException(HRESULT hr)
		:std::runtime_error(HrToString(hr)), m_hr(hr){}
private:
	HRESULT m_hr;
};

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw HrException(hr);
	}
}
#endif

void HelloTriangle::LoadPipeline()
{
	WNDCLASSEX winClassMessage{};
	winClassMessage.cbSize = sizeof(WNDCLASSEX);
	winClassMessage.style = CS_HREDRAW | CS_VREDRAW;
	winClassMessage.lpfnWndProc = WindowProcedure;
	winClassMessage.hInstance = GetModuleHandle(0);
	winClassMessage.lpszClassName = L"MainWindow";

	RegisterClassEx(&winClassMessage);
	
	RECT wrc{ 0,0,m_Width, m_Height };
	m_Window = CreateWindow(winClassMessage.lpszClassName,
		L"TriangleWindow", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		wrc.right - wrc.left, wrc.bottom - wrc.top, nullptr, nullptr,
		winClassMessage.hInstance, nullptr
	);
#ifdef _DEBUG
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
#endif
		ComPtr<IDXGIFactory4> factory;
		ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
		HRESULT hardwareResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));
		if (FAILED(hardwareResult))
		{
			ComPtr<IDXGIAdapter> pwrapAdapter;
			ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&pwrapAdapter)));
			ThrowIfFailed(D3D12CreateDevice(pwrapAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device)));
		}

		D3D12_COMMAND_QUEUE_DESC queueDesc{};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf())));

		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;   //60 / 1
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = swapChainBufferCount;
		swapChainDesc.OutputWindow = m_Window;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		//attention swap Chain need command queue to reflash
		ThrowIfFailed(factory->CreateSwapChain(
			m_CommandQueue.Get(),
			&swapChainDesc,
			m_SwapChain.GetAddressOf()
		));

		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
		descriptorHeapDesc.NumDescriptors = swapChainBufferCount;
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; //shader can't use
		descriptorHeapDesc.NodeMask = 0;
		
		ThrowIfFailed(m_Device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(m_DescriptorHeap.GetAddressOf())));

		m_RTVDescriptorMaxCount = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorPtr(m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT i = 0; i < (UINT)swapChainBufferCount; i++)
		{
			ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_SwapChainBuffer[i].GetAddressOf())));
			m_Device->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, descriptorPtr);
			descriptorPtr.Offset(1, m_RTVDescriptorMaxCount);
		}
}
