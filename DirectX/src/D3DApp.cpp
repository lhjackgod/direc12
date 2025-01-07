#include "D3DApp.h"
#include <assert.h>
#include <stdio.h>
D3DApp* D3DApp::mApp = nullptr;

D3DApp::D3DApp(HINSTANCE hInstance)
	:m_AppInstance(hInstance)
{
	assert(mApp == nullptr);
	mApp = this;
}

D3DApp::~D3DApp()
{
	if (m_Device != nullptr)
		FlushCommandQueue();
}

bool D3DApp::Initialize()
{
	if (!InitMainWindow())
		return false;
	if (!InitDirectx3D())
		return false;

	OnResize();
	
	return true;
}

LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void D3DApp::FlushCommandQueue()
{
	m_CurrentFence++;

	ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence));

	if (m_Fence->GetCompletedValue() < m_CurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::DepthStencilView()
{
	return m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

ID3D12Resource* D3DApp::CurrentBackBuffer() const
{
	return m_SwapchainBuffers[m_CurrentBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		m_CurrentBackBuffer,
		m_RTVDescriptorSize
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::DepthStencileView() const
{
	return m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

int D3DApp::Run()
{
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			Update();
			
			Draw();
		}
	}

	return (int)msg.wParam;
}

void D3DApp::OnResize()
{
	assert(m_Device);
	assert(m_SwapChain);
	assert(m_CommandAllocator);
	//flush everything before have finished
	FlushCommandQueue();
	ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), nullptr));

	for (int i = 0; i < SwapChainBufferCount; i++)
	{
		m_SwapchainBuffers[i].Reset();
	}
	m_DepthStencileBuffer.Reset();

	ThrowIfFailed(m_SwapChain->ResizeBuffers(SwapChainBufferCount,
		m_ClientWidth, m_ClientHeight,
		m_BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < SwapChainBufferCount; i++)
	{
		ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_SwapchainBuffers[i].GetAddressOf())));
		m_Device->CreateRenderTargetView(m_SwapchainBuffers[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_RTVDescriptorSize);
	}

	D3D12_RESOURCE_DESC depthResouceDesc{};
	depthResouceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResouceDesc.Alignment = 0;
	depthResouceDesc.Width = m_ClientWidth;
	depthResouceDesc.Height = m_ClientHeight;
	depthResouceDesc.DepthOrArraySize = 1;
	depthResouceDesc.MipLevels = 1;
	depthResouceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthResouceDesc.SampleDesc.Count = 1;
	depthResouceDesc.SampleDesc.Quality = 0;
	depthResouceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthResouceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	optClear.Format = m_DepthStencilFormat;
	ThrowIfFailed(m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE, &depthResouceDesc, D3D12_RESOURCE_STATE_COMMON, &optClear, IID_PPV_ARGS(m_DepthStencileBuffer.GetAddressOf())));

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc{};
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Format = m_DepthStencilFormat;
	depthStencilDesc.Texture2D.MipSlice = 0;

	m_Device->CreateDepthStencilView(m_DepthStencileBuffer.Get(), &depthStencilDesc, DepthStencilView());

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencileBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	// Execute the resize commands.
	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		// Wait until resize is complete.
	FlushCommandQueue();

	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(m_ClientWidth);
	m_ScreenViewport.Height = static_cast<float>(m_ClientHeight);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;

	m_ScissorRect = { 0, 0, m_ClientWidth, m_ClientHeight };
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return D3DApp::getApp()->MsgProc(hwnd, msg, wParam, lParam);
}

bool D3DApp::InitMainWindow()
{
	WNDCLASS wndClassEx{};

	wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
	wndClassEx.lpfnWndProc = MainWndProc;
	wndClassEx.cbClsExtra = 0;
	wndClassEx.cbWndExtra = 0;
	wndClassEx.hInstance = m_AppInstance;
	wndClassEx.hIcon = LoadIcon(0, IDI_APPLICATION);
	wndClassEx.hCursor = LoadCursor(0, IDC_ARROW);
	wndClassEx.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wndClassEx.lpszMenuName = 0;
	wndClassEx.lpszClassName = L"MainWindow";

	if (!RegisterClass(&wndClassEx))
	{
		MessageBox(0, L"RegisterClass Failed", 0, 0);
		return false;
	}

	RECT R = { 0, 0, m_ClientWidth, m_ClientHeight };
	
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false); //adjust the window rect
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	m_Window = CreateWindow(wndClassEx.lpszClassName, L"FirstProgram", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_AppInstance, 0);
	if (!m_Window)
	{
		MessageBox(0, L"CreateWindow Failed", 0, 0);
		return false;
	}

	ShowWindow(m_Window, SW_SHOW);
	UpdateWindow(m_Window);
	return true;
}

bool D3DApp::InitDirectx3D()
{
#if defined(DEBUG) || defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debug;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf())));
		debug->EnableDebugLayer();
	}
#endif
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(m_Factory.GetAddressOf())));

	HRESULT isokDevice = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.GetAddressOf()));
	if (FAILED(isokDevice))
	{
		ComPtr<IDXGIAdapter> softAdapter;
		ThrowIfFailed(m_Factory->EnumWarpAdapter(IID_PPV_ARGS(softAdapter.GetAddressOf())));

		ThrowIfFailed(D3D12CreateDevice(softAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.GetAddressOf())));
	}

	ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf())));
	
	m_CBVSRVUAVSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_RTVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DSVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaSupport{};
	msaaSupport.Format = m_BackBufferFormat;
	msaaSupport.SampleCount = 4;
	msaaSupport.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msaaSupport.NumQualityLevels = 0;
	ThrowIfFailed(m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaSupport,
		sizeof(msaaSupport)
	));
	
	m4xMsaaQuality = msaaSupport.NumQualityLevels;
	assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

#if defined(DEBUG) || defined(_DEBUG)
	{
		LogAdapter();
	}
#endif

	CreateCommandObjects();
	
	createSwapChain();
	
	createRtvAndDsvDescriptorHeaps();
	return true;
}

void D3DApp::LogAdapter()
{
	IDXGIAdapter* adapter;
	UINT i = 0;
	std::vector<IDXGIAdapter*> adapterList;
	while (m_Factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC adapterDesc{};
		adapter->GetDesc(&adapterDesc);

		std::wstring text = L"***Adapter: ";
		text += adapterDesc.Description;
		text += L"\n";
		
		OutputDebugString(text.c_str());

		adapterList.push_back(adapter);
		++i;
	}
	
	for (size_t i = 0; i < adapterList.size(); i++)
	{
		LogAdapterOutputs(adapterList[i]);
		adapterList[i]->Release();
	}
}

void D3DApp::LogAdapterOutputs(IDXGIAdapter* adapter)
{
	UINT i = 0;
	IDXGIOutput* output;
	while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC outputDesc{};
		output->GetDesc(&outputDesc);
		std::wstring text = L"***output: ";
		text += outputDesc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());

		LogOutputDisplayModes(output, m_BackBufferFormat);

		output->Release();
		i++;
	}
}

void D3DApp::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flag = 0;
	output->GetDisplayModeList(format, flag, &count, nullptr);

	std::vector<DXGI_MODE_DESC> modeList(count);
	output->GetDisplayModeList(format, flag, &count, modeList.data());

	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text =
			L"Width = " + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";
		OutputDebugString(text.c_str());
	}
}

void D3DApp::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_Device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf())));

	ThrowIfFailed(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocator.GetAddressOf())));

	ThrowIfFailed(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf())));
	m_CommandList->Close();
}

void D3DApp::createSwapChain()
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_ClientWidth;
	swapChainDesc.BufferDesc.Height = m_ClientHeight;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.Format = m_BackBufferFormat;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	swapChainDesc.SampleDesc.Count =  1;
	swapChainDesc.SampleDesc.Quality =  0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = SwapChainBufferCount;
	swapChainDesc.OutputWindow = m_Window;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ThrowIfFailed(m_Factory->CreateSwapChain(m_CommandQueue.Get(), &swapChainDesc, m_SwapChain.GetAddressOf()));
}

void D3DApp::createRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptorDesc{};
	descriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorDesc.NumDescriptors = SwapChainBufferCount;
	descriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descriptorDesc.NodeMask = 0;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&descriptorDesc, IID_PPV_ARGS(m_RtvDescriptorHeap.GetAddressOf())));
	
	D3D12_DESCRIPTOR_HEAP_DESC DsvDescriptorDesc{};
	DsvDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DsvDescriptorDesc.NumDescriptors = 1;
	DsvDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DsvDescriptorDesc.NodeMask = 0;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&DsvDescriptorDesc, IID_PPV_ARGS(m_DsvDescriptorHeap.GetAddressOf())));
}

void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw MyHRESULTFailed(hr);
	}
}
