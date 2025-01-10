#include "HelloTriangle.h"
#include <HelloTriangle/HelloTriangleWin.h>
void HelloTriangle::OnInit()
{
	LoadPipeline();
	LoadAsserts();
}

void HelloTriangle::OnDestroy()
{
	waitForFinish();
	CloseHandle(m_FenceEvent);
}

void HelloTriangle::OnRender()
{
	populateCommandList();

	ID3D12CommandList* ppCommandList[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(ppCommandList), ppCommandList);
	ThrowIfFiled(m_SwapChain->Present(1, 0));

	waitForFinish();
}

void HelloTriangle::OnUpdate()
{
}

void HelloTriangle::DealWithCommandLineArgs(int argc, WCHAR* argv[])
{
	for (int i = 1; i < argc; i++)
	{
		if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0 ||
			_wcsnicmp(argv[i], L"/warp", wcslen(argv[i])) == 0)
		{
			m_UseWarpDevice = true;
		}
	}
}

void HelloTriangle::LoadPipeline()
{
	UINT dxgiFactoryFlags = 0;
#if defined(DEBUG) || defined (_DEBUG)
	ComPtr<ID3D12Debug> debug;
	ThrowIfFiled(D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf())));
	debug->EnableDebugLayer();
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	ComPtr<IDXGIFactory4> factory;
	ThrowIfFiled(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));
	if (m_UseWarpDevice)
	{
		ComPtr<IDXGIAdapter> WrapAdapter;
		ThrowIfFiled(factory->EnumWarpAdapter(IID_PPV_ARGS(WrapAdapter.GetAddressOf())));

		ThrowIfFiled(D3D12CreateDevice(WrapAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.GetAddressOf())));
	}
	else
	{
		ThrowIfFiled(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.GetAddressOf())));
	}
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFiled(m_Device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf())));

	DXGI_SWAP_CHAIN_DESC1 swapChainDes = {};
	swapChainDes.BufferCount = FramBufferCount;
	swapChainDes.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDes.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDes.Height = m_Height;
	swapChainDes.Width = m_Width;
	swapChainDes.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDes.SampleDesc.Count = 1;
	swapChainDes.SampleDesc.Quality = 0;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFiled(factory->CreateSwapChainForHwnd(m_CommandQueue.Get(),
		HelloTriangleWin::getWnd(),
		&swapChainDes,
		nullptr, nullptr, swapChain.GetAddressOf()));
	ThrowIfFiled(swapChain.As(&m_SwapChain));
	m_FramIndex = m_SwapChain->GetCurrentBackBufferIndex();

	m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDesc.NumDescriptors = FramBufferCount;
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		ThrowIfFiled(m_Device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(m_DescriptorHeap.GetAddressOf())));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptorHand(m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		for (int i = 0; i < FramBufferCount; i++)
		{
			ThrowIfFiled(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_RenderBuffer[i].GetAddressOf())));
			m_Device->CreateRenderTargetView(m_RenderBuffer[i].Get(), nullptr, rtvDescriptorHand);
			rtvDescriptorHand.Offset(1, m_RtvDescriptorSize);
		}
	}

	ThrowIfFiled(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocator.GetAddressOf())));
}

void HelloTriangle::LoadAsserts()
{
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFiled(D3D12SerializeRootSignature(&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1, signature.GetAddressOf(), error.GetAddressOf()));
	ThrowIfFiled(m_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.GetAddressOf())));

	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

#if defined(DEBUG) || defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	ThrowIfFiled(D3DCompileFromFile(L"src/HelloTriangle/shader/shader.hlsl",
		nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, vertexShader.GetAddressOf(), nullptr));
	ThrowIfFiled(D3DCompileFromFile(L"src/HelloTriangle/shader/shader.hlsl",
		nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, pixelShader.GetAddressOf(), nullptr));

	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.NodeMask = 0;
	psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };
	psoDesc.pRootSignature = m_RootSignature.Get();
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	ThrowIfFiled(m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_PipelineState.GetAddressOf())));

	m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf()));
	ThrowIfFiled(m_CommandList->Close());
	m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf()));
	m_FenceValue = 1;

	m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_FenceEvent == nullptr)
	{
		ThrowIfFiled(HRESULT_FROM_WIN32(GetLastError()));
	}
}

void HelloTriangle::waitForFinish()
{
	const UINT64 fence = m_FenceValue;

	ThrowIfFiled(m_CommandQueue->Signal(m_Fence.Get(), m_FenceValue));
	m_FenceValue++;
	if (m_Fence->GetCompletedValue() < fence)
	{
		ThrowIfFiled(m_Fence->SetEventOnCompletion(fence, m_FenceEvent));
		WaitForSingleObject(m_FenceEvent, INFINITE);
	}
	m_FramIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

void HelloTriangle::populateCommandList()
{
	ThrowIfFiled(m_CommandAllocator->Reset());
	ThrowIfFiled(m_CommandList->Reset(m_CommandAllocator.Get(), m_PipelineState.Get()));

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderBuffer[m_FramIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	float ClearColor[4]{ 0.0f, 0.2f, 0.4f, 1.0f };
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_FramIndex, m_RtvDescriptorSize);
	m_CommandList->ClearRenderTargetView(handle, ClearColor, 0, nullptr);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderBuffer[m_FramIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFiled(m_CommandList->Close());
}
