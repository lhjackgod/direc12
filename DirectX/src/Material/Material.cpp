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

		D3D12_DESCRIPTOR_HEAP_DESC srvHeap{};
		srvHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeap.NumDescriptors = 1;
		srvHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFiled(m_Device->CreateDescriptorHeap(&srvHeap, IID_PPV_ARGS(m_SrvDescriptorHeap.GetAddressOf())));

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

	D3D12_FEATURE_DATA_ROOT_SIGNATURE rootSignatureFeature{};
	rootSignatureFeature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE,
		&rootSignatureFeature, sizeof(rootSignatureFeature))))
	{
		rootSignatureFeature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}


	CD3DX12_DESCRIPTOR_RANGE1 desciptorRanges[1];
	desciptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

	CD3DX12_ROOT_PARAMETER1 rootParamter[1];
	rootParamter[0].InitAsDescriptorTable(1, &desciptorRanges[0], D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_STATIC_SAMPLER_DESC staticSampler{};
	staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	staticSampler.MipLODBias = 0;
	staticSampler.MaxAnisotropy = 0;
	staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	staticSampler.MinLOD = 0.0f;
	staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
	staticSampler.ShaderRegister = 0;
	staticSampler.RegisterSpace = 0;
	staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionRootSignatureDesc{};
	versionRootSignatureDesc.Init_1_1(_countof(rootParamter), rootParamter,
		1, &staticSampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSignature;
	ComPtr<ID3DBlob> error;
	
	D3DX12SerializeVersionedRootSignature(&versionRootSignatureDesc,
		rootSignatureFeature.HighestVersion, rootSignature.GetAddressOf(),
		error.GetAddressOf());
	ThrowIfFiled(m_Device->CreateRootSignature(0,
		rootSignature->GetBufferPointer(),
		rootSignature->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature.GetAddressOf())));

	D3D12_INPUT_ELEMENT_DESC inputElement[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

#if defined(DEBUG) || defined(_DEBUG)
	
	UINT compileFLag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFLag = 0;
#endif

	ComPtr<ID3DBlob> vShader;
	ComPtr<ID3DBlob> pShader;

	ThrowIfFiled(D3DCompileFromFile(L"src/Material/shader/Material.hlsl", nullptr,
		nullptr, "VSMain", "vs_5_0", compileFLag, 0, vShader.GetAddressOf(), nullptr));
	ThrowIfFiled(D3DCompileFromFile(L"src/Material/shader/Material.hlsl", nullptr,
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
		{{0.0f, 0.5f, 0.0f}, { 0.5f, 0.0f }},
		{{0.5f, -0.5f, 0.0f}, { 1.0f, 1.0f }},
		{{-0.5f, -0.5f, 0.0f}, { 0.0f, 1.0f }}
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

	ComPtr<ID3D12Resource> m_TextureBuffer;
	{
		//texture
		D3D12_RESOURCE_DESC textureDesc{};
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureDesc.Width = m_ImageWidth;
		textureDesc.Height = m_ImageHeight;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.MipLevels = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ThrowIfFiled(m_Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(m_gpuTextureResource.GetAddressOf())
		));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_gpuTextureResource.Get(),
			0, 1);

		ThrowIfFiled(m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(m_TextureBuffer.GetAddressOf())));

		std::vector<UINT8> texture = GenerateTextureData();

		D3D12_SUBRESOURCE_DATA textureData{};
		textureData.pData = texture.data();
		textureData.RowPitch = m_ImageWidth * m_TexturePixelSize;
		textureData.SlicePitch = m_ImageHeight * textureData.RowPitch;
		UpdateSubresources(m_CommandList.Get(), m_gpuTextureResource.Get(), m_TextureBuffer.Get(),
			0, 0, 1, &textureData);
		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_gpuTextureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		));
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;
		m_Device->CreateShaderResourceView(m_gpuTextureResource.Get(), &srvDesc,
			m_SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}

	ThrowIfFiled(m_CommandList->Close());

	ID3D12CommandList* commandLists[] = {m_CommandList.Get()};
	m_CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

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

	

	float clearColor[] = { 0.0f, 0.4f, 0.2f, 1.0f };

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescritorHeap->GetCPUDescriptorHandleForHeapStart(), m_CurrentBackFrambufferIndex, RTVTOTALSIZE);


	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_SrvDescriptorHeap.Get() };
	m_CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	m_CommandList->SetGraphicsRootDescriptorTable(0, m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	m_CommandList->RSSetViewports(1, &m_ViewPort);

	m_CommandList->RSSetScissorRects(1, &m_SsiorRect);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Frambuffer[m_CurrentBackFrambufferIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	m_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

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

std::vector<UINT8> Material::GenerateTextureData()
{
	const UINT rowPitch = m_ImageWidth  * m_TexturePixelSize;
	const UINT cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
	const UINT cellHeight = m_ImageWidth >> 3;    // The height of a cell in the checkerboard texture.
	const UINT textureSize = rowPitch * m_ImageHeight;

	std::vector<UINT8> data(textureSize);
	UINT8* pData = &data[0];

	for (UINT n = 0; n < textureSize; n += m_TexturePixelSize)
	{
		UINT x = n % rowPitch;
		UINT y = n / rowPitch;
		UINT i = x / cellPitch;
		UINT j = y / cellHeight;

		if (i % 2 == j % 2)
		{
			pData[n] = 0x00;        // R
			pData[n + 1] = 0x00;    // G
			pData[n + 2] = 0x00;    // B
			pData[n + 3] = 0xff;    // A
		}
		else
		{
			pData[n] = 0xff;        // R
			pData[n + 1] = 0xff;    // G
			pData[n + 2] = 0xff;    // B
			pData[n + 3] = 0xff;    // A
		}
	}

	return data;
}
