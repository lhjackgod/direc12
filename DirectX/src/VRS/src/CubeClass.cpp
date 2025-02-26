#include "CubeClass.h"
#include <d3dx12.h>
#include "WinApp.h"
#include <array>
#include <DirectXColors.h>
#include <chrono>
#define Clamp(x, a, b) max(a, min(x, b)) 
DirectX::XMFLOAT4 MyColor::Red = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
DirectX::XMFLOAT4 MyColor::Black = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
DirectX::XMFLOAT4 MyColor::Blue = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
DirectX::XMFLOAT4 MyColor::Green = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
DirectX::XMFLOAT4 MyColor::White = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
DirectX::XMFLOAT4 MyColor::Yellow = DirectX::XMFLOAT4(1.f, 1.f, 0.f, 1.f);
DirectX::XMFLOAT4 MyColor::Cyan = DirectX::XMFLOAT4(0.f, 1.f, 1.f, 1.f);
DirectX::XMFLOAT4 MyColor::Magenta = DirectX::XMFLOAT4(1.f, 0.f, 1.f, 1.f);
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
		float r = 5.0f;
		float x = cosf(phi) * cosf(theta) * r;
		float y = sinf(phi) * r;
		float z = cosf(phi) * sinf(theta) * r;
		m_MainCamera.setCameraPos(DirectX::XMVectorSet(x, y, z, 1.0f));
		DirectX::XMStoreFloat4x4(&m_MVPMatrix.mvp,
			DirectX::XMMatrixTranspose(m_CubeModel * m_MainCamera.getViewMatrix() * m_MainCamera.getPerspective()));
	}

	void HelloTriangle::UpdateBuffer(ComPtr<ID3D12Resource>& defaultBuffer, ComPtr<ID3D12Resource>& uploadBuffer, UINT byteSize,
		void* data)
	{
		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));
		D3D12_SUBRESOURCE_DATA subResourceData{};
		subResourceData.pData = data;
		subResourceData.RowPitch = byteSize;
		subResourceData.SlicePitch = subResourceData.RowPitch;
		UpdateSubresources(m_CommandList.Get(), defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
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

	void HelloTriangle::OnMouseOn(WPARAM wParam, int xPos, int yPos)
	{
		currentMouseLoX = xPos;
		currentMouseLoY = yPos;
		SetCapture(WinApp::getHwnd());
	}

	void HelloTriangle::OnMouseUp(WPARAM wParam, int xPos, int yPos)
	{
		ReleaseCapture();
	}

	void HelloTriangle::OnMouseMove(WPARAM wParam, int xPos, int yPos)
	{
		if ((wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON)) != 0)
		{
			float dx = DirectX::XMConvertToRadians(currentMouseLoX - xPos) * 0.25f;
			float dy = DirectX::XMConvertToRadians(currentMouseLoY - yPos) * 0.25f;
			theta += dx;
			phi += dy;
			//std::max(0.1f, std::min(x, 3.1416f - 0.1f))
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
		createDevice(factory);
		createCommandQueue();
		createCommandAllocate();
		createSwapChain(factory);
		createFence();
		createDescriptorHeap();
		createRTV();
		createDSV();
		m_FramIndex = m_SwapChain->GetCurrentBackBufferIndex();

		m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_ViewPort.Height = m_Height;
		m_ViewPort.MaxDepth = 1.0f;
		m_ViewPort.MinDepth = 0.0f;
		m_ViewPort.TopLeftX = 0.0f;
		m_ViewPort.TopLeftY = 0.0f;
		m_ViewPort.Width = m_Width;

		m_Scissor.bottom = static_cast<LONG>(m_Height);
		m_Scissor.left = 0;
		m_Scissor.right = static_cast<LONG>(m_Width);
		m_Scissor.top = 0;
		ThrowIfFiled(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocator.GetAddressOf())));
	}
	void HelloTriangle::createDevice(ComPtr<IDXGIFactory4>& factory)
	{
		ThrowIfFiled(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.GetAddressOf())));
	}
	void HelloTriangle::createCommandQueue()
	{
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ThrowIfFiled(m_Device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf())));
	}
	void HelloTriangle::createCommandAllocate()
	{
		ThrowIfFiled(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_CommandAllocator.GetAddressOf())));
	}
	void HelloTriangle::createSwapChain(ComPtr<IDXGIFactory4>& factory)
	{
		ComPtr<IDXGISwapChain1> pSwapChain;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
		swapChainDesc.BufferCount = FramBufferCount;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.Height = m_Height;
		swapChainDesc.Width = m_Width;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		ThrowIfFiled(factory->CreateSwapChainForHwnd(m_CommandQueue.Get(), WinApp::getHwnd(),
		&swapChainDesc, nullptr, nullptr, pSwapChain.GetAddressOf()
	));
		ThrowIfFiled(pSwapChain.As(&m_SwapChain));
	}
	void HelloTriangle::createFence()
	{
		m_FenceValue = 1;
		m_FenceEvent = CreateEvent(nullptr, false, false, nullptr);
		ThrowIfFiled(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf())));
		if(m_FenceEvent == nullptr)
		{
			ThrowIfFiled(HRESULT_FROM_WIN32(GetLastError()));
		}
	}
	void HelloTriangle::createDescriptorHeap()
	{
		m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeap{};
		rtvDescriptorHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvDescriptorHeap.NumDescriptors = FramBufferCount;
		rtvDescriptorHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		ThrowIfFiled(m_Device->CreateDescriptorHeap(&rtvDescriptorHeap,
		IID_PPV_ARGS(m_RtvDescriptorHeap.GetAddressOf())));

		m_DsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc{};
		dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvDescriptorHeapDesc.NumDescriptors = 1;
		dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		ThrowIfFiled(m_Device->CreateDescriptorHeap(&dsvDescriptorHeapDesc,
		IID_PPV_ARGS(m_DsvDescriptorHeap.GetAddressOf())));
		
		m_CbvSrvUavDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc{};
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NumDescriptors = 1;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		ThrowIfFiled(m_Device->CreateDescriptorHeap(&cbvHeapDesc,
			IID_PPV_ARGS(m_ConstBufferViewHeap.GetAddressOf())));
	}
	void HelloTriangle::createRTV()
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		for(int i = 0; i < FramBufferCount; i++)
		{
			ThrowIfFiled(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_RenderBuffer[i].GetAddressOf())));
			m_Device->CreateRenderTargetView(m_RenderBuffer[i].Get(),
		nullptr, rtvHandle);
		rtvHandle.Offset(1, m_RtvDescriptorSize);
		}
	}
	void HelloTriangle::createDSV()
	{
		D3D12_RESOURCE_DESC dsvResourceDesc{};
		dsvResourceDesc.Alignment = 0;
		dsvResourceDesc.DepthOrArraySize = 1;
		dsvResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		dsvResourceDesc.Width = m_Width;
		dsvResourceDesc.Height = m_Height;
		dsvResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		dsvResourceDesc.MipLevels = 1;
		dsvResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		dsvResourceDesc.SampleDesc.Count = 1;
		dsvResourceDesc.SampleDesc.Quality = 0;
		D3D12_CLEAR_VALUE clearValue{};
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;
		clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		ThrowIfFiled(m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	D3D12_HEAP_FLAG_NONE, &dsvResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(m_DsvBuffer.GetAddressOf())));

		m_Device->CreateDepthStencilView(m_DsvBuffer.Get(), nullptr, m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	}
	void HelloTriangle::LoadAsserts()
	{
		createRootSignature();
		createShader();
		createVertexBufferAndInputLayout();
		createPipelineState();
		m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), m_PipelineState.Get(), IID_PPV_ARGS(m_CommandList.GetAddressOf()));
		createIndexBuffer();
		createConstBuffer();
		createConstBufferView();
		ThrowIfFiled(m_CommandList->Close());
		ID3D12CommandList* pCommandLists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(pCommandLists), pCommandLists);
		waitForFinish();
		m_MainCamera = Camera(m_Width, m_Height, 0.1f, 100.0f, DirectX::XMVectorSet(0.f, 0.f, -5.0f, 1.0f));
	}
	void HelloTriangle::createRootSignature()
	{
		CD3DX12_DESCRIPTOR_RANGE1 descriptorRange[1];
		descriptorRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0);
		CD3DX12_ROOT_PARAMETER1 rootParamter[1];
		rootParamter[0].InitAsDescriptorTable(1, &descriptorRange[0]);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
		rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if(FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, 
		&rootSignatureDesc, sizeof(rootSignatureDesc))))
		{
			rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}
		rootSignatureDesc.Init_1_1(1, &rootParamter[0], 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFiled(D3D12SerializeVersionedRootSignature(&rootSignatureDesc,
		signature.GetAddressOf(), error.GetAddressOf()));
		ThrowIfFiled(m_Device->CreateRootSignature(0, signature->GetBufferPointer(),
	signature->GetBufferSize(),IID_PPV_ARGS(m_RootSignature.GetAddressOf())));
	}
	void HelloTriangle::createShader()
	{
		UINT compileFlags = 0;
		compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
		ThrowIfFiled(D3DCompileFromFile(L"src/shader/cube.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, m_VertexShader.GetAddressOf(), nullptr));
		ThrowIfFiled(D3DCompileFromFile(L"src/shader/cube.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, m_PixelShader.GetAddressOf(), nullptr));
	}
	void HelloTriangle::createVertexBufferAndInputLayout()
	{
		Vertex vertexs[]
		{
			{{-1.0f, -1.0f, -1.0f}, MyColor::White},
			{{-1.0f, +1.0f, -1.0f}, MyColor::Black},
			{{+1.0f, +1.0f, -1.0f}, MyColor::Red},
			{{+1.0f, -1.0f, -1.0f}, MyColor::Green},

			{{-1.0f, -1.0f, +1.0f}, MyColor::Blue},
			{{-1.0f, +1.0f, +1.0f}, MyColor::Yellow},
			{{+1.0f, +1.0f, +1.0f}, MyColor::Cyan},
			{{+1.0f, -1.0f, +1.0f}, MyColor::Magenta}
		};
		UINT vertexByte = sizeof(vertexs);
		ThrowIfFiled(m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexByte), D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(m_VertexBuffer.GetAddressOf())));
		m_VertexBufferSize = sizeof(vertexs);
		UINT8* vertexPtr;
		D3D12_RANGE range{ 0,0 };
		ThrowIfFiled(m_VertexBuffer->Map(0, &range, reinterpret_cast<void**>(&vertexPtr)));
		memcpy(vertexPtr, vertexs, sizeof(vertexs));
		m_VertexBuffer->Unmap(0, nullptr);
	}

	void HelloTriangle::createIndexBuffer()
	{
		UINT indices[] = {
			// 前
		0, 1, 2,
		0, 2, 3,

		// 后
		4, 6, 5,
		4, 7, 6,

		// 左
		4, 5, 1,
		4, 1, 0,

		// 右
		3, 2, 6,
		3, 6, 7,

		// 上
		1, 5, 6,
		1, 6, 2,

		// 下
		4, 0, 3,
		4, 3, 7
		};
		UINT indexBufferSize = m_IndexBufferSize = sizeof(indices);
		createDefaultBuffer(indices, indexBufferSize, m_IndexBuffer, m_UploadBuffer);
	}

	void HelloTriangle::createConstBuffer()
	{
		DirectX::XMStoreFloat4x4(&m_MVPMatrix.mvp, DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()));
		UINT constBufferSize = CalcConstBufferByteSize(sizeof(ConstBuffer));
		ThrowIfFiled(m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(constBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_ConstBuffer.GetAddressOf())));

		ThrowIfFiled(m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(constBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_ConstBufferUpload.GetAddressOf())));
	}

	void HelloTriangle::createConstBufferView()
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC constBufferView{};
		constBufferView.BufferLocation = m_ConstBuffer->GetGPUVirtualAddress();
		constBufferView.SizeInBytes = CalcConstBufferByteSize(sizeof(ConstBuffer));
		m_Device->CreateConstantBufferView(&constBufferView,
			m_ConstBufferViewHeap->GetCPUDescriptorHandleForHeapStart());
	}

	D3D12_INDEX_BUFFER_VIEW HelloTriangle::getIndexBufferView()
	{
		D3D12_INDEX_BUFFER_VIEW indexBufferView{};
		indexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
		indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		indexBufferView.SizeInBytes = m_IndexBufferSize;
		return indexBufferView;
	}

	void HelloTriangle::createDefaultBuffer(const void* data, const UINT byteSize, ComPtr<ID3D12Resource>& defaultBuffer,
		ComPtr<ID3D12Resource>& upLoad)
	{
		ThrowIfFiled(m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(defaultBuffer.GetAddressOf())
		));
		
		ThrowIfFiled(m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(byteSize), D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(upLoad.GetAddressOf())));

		D3D12_SUBRESOURCE_DATA subResourceData{};
		subResourceData.pData = data;
		subResourceData.RowPitch = byteSize;
		subResourceData.SlicePitch = subResourceData.RowPitch;

		UpdateSubresources(m_CommandList.Get(), defaultBuffer.Get(), upLoad.Get(),
			0, 0, 1, &subResourceData);
		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
	}

	D3D12_VERTEX_BUFFER_VIEW HelloTriangle::getVertexBufferView()
	{
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
		vertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = m_VertexBufferSize;
		vertexBufferView.StrideInBytes = sizeof(Vertex);
		return vertexBufferView;
	}
	void HelloTriangle::createPipelineState()
	{
		D3D12_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
		psoDesc.pRootSignature = m_RootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_VertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_PixelShader.Get());
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		ThrowIfFiled(m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_PipelineState.GetAddressOf())));
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

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_FramIndex, m_RtvDescriptorSize);
		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderBuffer[m_FramIndex].Get(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		m_CommandList->OMSetRenderTargets(1, &rtvHandle, true, &m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		m_CommandList->RSSetViewports(1, &m_ViewPort);
		m_CommandList->RSSetScissorRects(1, &m_Scissor);
		float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		m_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		m_CommandList->ClearDepthStencilView(m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			D3D12_CLEAR_FLAG_DEPTH, 1.0, 0, 0, nullptr);
		m_CommandList->IASetVertexBuffers(0, 1, &getVertexBufferView());
		m_CommandList->IASetIndexBuffer(&getIndexBufferView());
		m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
		UpdateBuffer(m_ConstBuffer, m_ConstBufferUpload, CalcConstBufferByteSize(sizeof(ConstBuffer)), reinterpret_cast<void*>(&m_MVPMatrix));
		ID3D12DescriptorHeap* ppHeaps[]{ m_ConstBufferViewHeap.Get() };
		m_CommandList->SetDescriptorHeaps(1, ppHeaps);
		m_CommandList->SetGraphicsRootDescriptorTable(0, m_ConstBufferViewHeap->GetGPUDescriptorHandleForHeapStart());
		m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_CommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderBuffer[m_FramIndex].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		ThrowIfFiled(m_CommandList->Close());
	}
