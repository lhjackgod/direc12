#include "Material.h"
#include "Material/MaterialWinApplication.h"
void Material::OnInit()
{
	LoadPipeline();
	LoadAssert();
}

void Material::OnRender()
{
	waitForFinish();

	populateCommandList();
	ID3D12CommandList* commandList[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(commandList), commandList);
	ThrowIfFiled(m_SwapChain->Present(1, 0));
	waitForFinish();
}

void Material::OnUpdate()
{
}

void Material::OnDestroy()
{
	waitForFinish();
	CloseHandle(m_FenceEvent);
}

void Material::DealWithCommand(int argc, WCHAR* argv[])
{
	for (int i = 1; i < argc; i++)
	{
		if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0)
		{
			useWarp = true;
		}
	}
}

void Material::LoadPipeline()
{
	UINT dxgiFactoyFlag = 0;
#if defined(DEBUG) || defined(_DEBUG)
	ComPtr<ID3D12Debug> debugLayout;
	ThrowIfFiled(D3D12GetDebugInterface(IID_PPV_ARGS(debugLayout.GetAddressOf())));
	debugLayout->EnableDebugLayer();
	dxgiFactoyFlag |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	ComPtr<IDXGIFactory7> factory;
	ThrowIfFiled(CreateDXGIFactory2(dxgiFactoyFlag, IID_PPV_ARGS(factory.GetAddressOf())));
	if (useWarp)
	{
		ComPtr<IDXGIAdapter> adapter;
		ThrowIfFiled(factory->EnumWarpAdapter(IID_PPV_ARGS(adapter.GetAddressOf())));
		ThrowIfFiled(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.GetAddressOf())));
	}
	else
	{
		ComPtr<IDXGIAdapter1> adapter;

		GetHardwareAdapter(adapter.GetAddressOf(), factory.Get(), true);
		ThrowIfFiled(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.GetAddressOf())));
	}

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ThrowIfFiled(m_Device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf())));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = m_Width;
	swapChainDesc.Height = m_Height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = framBufferCount;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFiled(factory->CreateSwapChainForHwnd(m_CommandQueue.Get(),
		MaterialWinApplication::getWin(), &swapChainDesc, nullptr, nullptr, swapChain.GetAddressOf()));
	ThrowIfFiled(swapChain.As(&m_SwapChain));

	m_CurrentBackFrambufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

	RTVTOTALSIZE = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	{
		D3D12_DESCRIPTOR_HEAP_DESC descritorHeap{};
		descritorHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descritorHeap.NumDescriptors = framBufferCount;
		descritorHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descritorHeap.NodeMask = 0;
		ThrowIfFiled(m_Device->CreateDescriptorHeap(&descritorHeap, IID_PPV_ARGS(m_rtvDescritorHeap.GetAddressOf())));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescritorHeap->GetCPUDescriptorHandleForHeapStart());

		for (UINT i = 0; i < framBufferCount; i++)
		{
			ThrowIfFiled(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_Frambuffer[i].GetAddressOf())));
			m_Device->CreateRenderTargetView(m_Frambuffer[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, RTVTOTALSIZE);
		}

	}

	ThrowIfFiled(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(m_CommandAllocator.GetAddressOf())));
}

void Material::LoadAssert()
{
	ThrowIfFiled(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf())));

	ThrowIfFiled(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf())));
	ThrowIfFiled(m_CommandList->Close());

	//create vertex buffer
	// create rootsignature
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFiled(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.GetAddressOf(), error.GetAddressOf()));
	ThrowIfFiled(m_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.GetAddressOf())));

	D3D12_INPUT_ELEMENT_DESC inputElement[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

#if defined(DEBUG) || defined(_DEBUG)
	
	UINT compileFLag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFLag = 0;
#endif

	ComPtr<ID3DBlob> vShader;
	ComPtr<ID3DBlob> pShader;

	ThrowIfFiled(D3DCompileFromFile(L"src/HelloTriangle/shader/shader.hlsl", nullptr,
		nullptr, "VSMain", "vs_5_0", compileFLag, 0, vShader.GetAddressOf(), nullptr));
	ThrowIfFiled(D3DCompileFromFile(L"src/HelloTriangle/shader/shader.hlsl", nullptr,
		nullptr, "PSMain", "ps_5_0", compileFLag, 0, pShader.GetAddressOf(), nullptr));
	
	//load pipeline
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gRaphicsPipelineStateDesc{};
	gRaphicsPipelineStateDesc.pRootSignature = m_RootSignature.Get();
	gRaphicsPipelineStateDesc.VS = CD3DX12_SHADER_BYTECODE(vShader.Get());
	gRaphicsPipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(pShader.Get());
	gRaphicsPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gRaphicsPipelineStateDesc.SampleMask = UINT_MAX;
	gRaphicsPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gRaphicsPipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
	gRaphicsPipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
	gRaphicsPipelineStateDesc.InputLayout = { inputElement, _countof(inputElement) };
	gRaphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gRaphicsPipelineStateDesc.NumRenderTargets = 1;
	gRaphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gRaphicsPipelineStateDesc.SampleDesc.Count = 1;
	gRaphicsPipelineStateDesc.SampleDesc.Quality = 0;

	ThrowIfFiled(m_Device->CreateGraphicsPipelineState(&gRaphicsPipelineStateDesc, IID_PPV_ARGS(m_PiplineSate.GetAddressOf())));

	Vertex vertexs[] =
	{
		{{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f ,1.0f, 1.0f}}
	};
	UINT vertexBufferSize = sizeof(vertexs);
	ThrowIfFiled(m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(m_VertexBuffer.GetAddressOf())));

	UINT8* cpuStart;
	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFiled(m_VertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&cpuStart)));
	memcpy(cpuStart, vertexs, vertexBufferSize);
	m_VertexBuffer->Unmap(0, nullptr);

	m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.SizeInBytes = vertexBufferSize;
	m_VertexBufferView.StrideInBytes = sizeof(Vertex);

	m_FenceValue = 1;
	m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_FenceEvent == nullptr)
	{
		ThrowIfFiled(HRESULT_FROM_WIN32(GetLastError()));
	}
	waitForFinish();
}

void Material::waitForFinish()
{
	const UINT64 FenceValue = m_FenceValue;
	ThrowIfFiled(m_CommandQueue->Signal(m_Fence.Get(), FenceValue));

	m_FenceValue++;

	if (m_Fence->GetCompletedValue() < FenceValue)
	{
		ThrowIfFiled(m_Fence->SetEventOnCompletion(FenceValue, m_FenceEvent));
		WaitForSingleObject(m_FenceEvent, INFINITE);
	}
	m_CurrentBackFrambufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

void Material::populateCommandList()
{
	ThrowIfFiled(m_CommandAllocator->Reset());
	ThrowIfFiled(m_CommandList->Reset(m_CommandAllocator.Get(), m_PiplineSate.Get()));

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Frambuffer[m_CurrentBackFrambufferIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	float clearColor[] = { 0.0f, 0.4f, 0.2f, 1.0f };

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescritorHeap->GetCPUDescriptorHandleForHeapStart(), m_CurrentBackFrambufferIndex, RTVTOTALSIZE);
	m_CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	m_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
	m_CommandList->RSSetViewports(1, &m_ViewPort);
	m_CommandList->RSSetScissorRects(1, &m_SsiorRect);

	m_CommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_CommandList->DrawInstanced(3, 1, 0, 0);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Frambuffer[m_CurrentBackFrambufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT));
	ThrowIfFiled(m_CommandList->Close());
}

void Material::GetHardwareAdapter(IDXGIAdapter1** adapter, IDXGIFactory7* factory, bool requestHightPerformanceAdapter)
{
	*adapter = nullptr;

	ComPtr<IDXGIAdapter1> pAdapter;
	if (SUCCEEDED(factory->QueryInterface(__uuidof(IDXGIFactory6), nullptr)))
	{
		for (
			UINT adapterIndex = 0;
			SUCCEEDED(factory->EnumAdapterByGpuPreference(
				adapterIndex, requestHightPerformanceAdapter ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE :
				DXGI_GPU_PREFERENCE_UNSPECIFIED, IID_PPV_ARGS(pAdapter.GetAddressOf())));
				adapterIndex++
			)
		{
			DXGI_ADAPTER_DESC1 adapterDesc;
			pAdapter->GetDesc1(&adapterDesc);

			if (adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
			{
				continue;
			}
			if (SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	if (pAdapter.Get() == nullptr)
	{
		for(UINT adapterIndex = 0; SUCCEEDED(factory->EnumAdapters1(adapterIndex, pAdapter.GetAddressOf()));
		++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 adapterDesc1;
			pAdapter->GetDesc1(&adapterDesc1);

			if (adapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}
			if (SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}
	*adapter = pAdapter.Detach();
}
