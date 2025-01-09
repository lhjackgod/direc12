#include "D3d12HelloWindow.h"
#include "d3dx12.h"
#include "HelloWindow/Win32Application.h"



HelloWindow::HelloWindow(UINT width, UINT height, std::wstring name)
	:m_FrambufferIndex(0), m_rtvDescriptorSize(0)
{
}

void HelloWindow::OnInit()
{
	LoadPipeline();
	LoadAsserts();
}

void HelloWindow::OnUpdate()
{
}

void HelloWindow::OnRender()
{
	PopulateCommandList();

	ID3D12CommandList* ppCommandLists[]{m_CommandList.Get()};
	m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	ThrowIfFailed(m_Swapchain->Present(1, 0));
	WaitForPreviousFrame();
}

void HelloWindow::OnDestroy()
{
	WaitForPreviousFrame();

	CloseHandle(m_FenceEvent);
}


void HelloWindow::LoadPipeline()
{
	UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG) || defined(DEBUG)
	{
		ComPtr<ID3D12Debug> DebugLayout;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(DebugLayout.GetAddressOf()))))
		{
			DebugLayout->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(m_Factory.GetAddressOf())));

	HRESULT m_CreateDeviceResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.GetAddressOf()));

	if (FAILED(m_CreateDeviceResult))
	{
		ComPtr<IDXGIAdapter> wrapAdapter;
		m_Factory->EnumWarpAdapter(IID_PPV_ARGS(wrapAdapter.GetAddressOf()));
		ThrowIfFailed(D3D12CreateDevice(wrapAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.GetAddressOf())));
	}
	m_rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	ThrowIfFailed(m_Device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf())));
	

	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc1{};
	dxgiSwapChainDesc1.Width = m_ClientWidth;
	dxgiSwapChainDesc1.Height = m_ClientHeight;
	dxgiSwapChainDesc1.Format = m_BackFormat;
	dxgiSwapChainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc1.BufferCount = FramCount;
	dxgiSwapChainDesc1.SampleDesc.Count = 1;
	dxgiSwapChainDesc1.SampleDesc.Quality = 0;
	dxgiSwapChainDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	
	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(m_Factory->CreateSwapChainForHwnd(m_CommandQueue.Get(),
		Win32Application::GetHwnd(), &dxgiSwapChainDesc1, nullptr, nullptr, swapChain.GetAddressOf()));

	ThrowIfFailed(swapChain.As(&m_Swapchain));
	m_FrambufferIndex = m_Swapchain->GetCurrentBackBufferIndex();

	D3D12_DESCRIPTOR_HEAP_DESC d3d12DescriptorHeadDesc{};
	d3d12DescriptorHeadDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3d12DescriptorHeadDesc.NumDescriptors = FramCount;
	d3d12DescriptorHeadDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ThrowIfFailed(m_Device->CreateDescriptorHeap(&d3d12DescriptorHeadDesc, IID_PPV_ARGS(m_RtvDescriptorHeap.GetAddressOf())));
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT n = 0; n < FramCount; n++)
	{
		ThrowIfFailed(m_Swapchain->GetBuffer(n, IID_PPV_ARGS(m_RenderTargets[n].GetAddressOf())));
		m_Device->CreateRenderTargetView(m_RenderTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}
	ThrowIfFailed(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_CommandAllocator.GetAddressOf())));
}

void HelloWindow::LoadAsserts()
{
	//create CommandList
	ThrowIfFailed(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf())));
	ThrowIfFailed(m_CommandList->Close());
	//create fence
	ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf())));
	m_FenceValue = 1;

	m_FenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (m_FenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
}

void HelloWindow::PopulateCommandList()
{
	ThrowIfFailed(m_CommandAllocator->Reset());
	ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), m_PipelineState.Get()));

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrambufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_FrambufferIndex, m_rtvDescriptorSize);
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrambufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(m_CommandList->Close());
}

void HelloWindow::WaitForPreviousFrame()
{
	const UINT64 fence = m_FenceValue;
	ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), fence));

	m_FenceValue++;
	if (m_Fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_Fence->SetEventOnCompletion(fence, m_FenceEvent));
		WaitForSingleObject(m_FenceEvent, INFINITE);
	}
	m_FrambufferIndex = m_Swapchain->GetCurrentBackBufferIndex();
	
}


