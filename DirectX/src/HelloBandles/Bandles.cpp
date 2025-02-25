#ifdef Bundle
#include "Bandles.h"
#include "HelloBandles/BandlesApp.h"
#include <array>
Bandles::Bandles(int width, int height, const WCHAR* winName)
    : m_Width(width), m_Height(height), m_WindowName(winName)
{
    m_Viewport.Height = m_Height;
    m_Viewport.MaxDepth = 1.0f;
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.TopLeftX = 0.0f;
    m_Viewport.TopLeftY = 0.0f;
    m_Viewport.Width = m_Width;

    m_Sissor.bottom = m_Height;
    m_Sissor.left = 0.0f;
    m_Sissor.right = m_Width;
    m_Sissor.top = 0.0f;
}

int Bandles::getWidth()
{
    return m_Width;
}

int Bandles::getHeight()
{
    return m_Height;
}

const WCHAR *Bandles::getName()
{
    return m_WindowName;
}

void Bandles::onInit()
{
    LoadPipeline();
    LoadAssert();
}

void Bandles::onRender()
{
    waitForFinish();
    begin = std::chrono::high_resolution_clock::now();
    fillOutCommandList();
    
    ID3D12CommandList* commandLists[]{m_CommandList.Get()};
    m_CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
    ThrowIfFiled(m_SwapChain->Present(1, 0));
    waitForFinish();
    end = std::chrono::high_resolution_clock::now();
    long long durationTime = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    std::cout << "Duration Time :" << durationTime <<std::endl;
}

void Bandles::onUpdate()
{
}

void Bandles::onDesctroy()
{
    waitForFinish();
    CloseHandle(m_FenceEvent);
}

void Bandles::dealWithCommandLiine(int argc, LPWSTR argv[])
{
    for(int i = 0; i < argc; i++)
    {
        if(_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0)
        {
            m_UseWarp = true;
        }
    }
}

void Bandles::LoadAssert()
{
    ThrowIfFiled(m_CommandAllocator->Reset());
    ThrowIfFiled(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
    m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf())));

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if(FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &rootSignatureDesc,
    sizeof(rootSignatureDesc))))
    {
        rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
    CD3DX12_DESCRIPTOR_RANGE1 descriptorRange[1];
    descriptorRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
        1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

    CD3DX12_ROOT_PARAMETER1 rootParam[1];
    rootParam[0].InitAsDescriptorTable(1, &descriptorRange[0], D3D12_SHADER_VISIBILITY_VERTEX);
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionRootSignatureDesc{};

    versionRootSignatureDesc.Init_1_1(_countof(rootParam), &rootParam[0], 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    D3DX12SerializeVersionedRootSignature(&versionRootSignatureDesc,
    rootSignatureDesc.Version, signature.GetAddressOf(), error.GetAddressOf());
    ThrowIfFiled(m_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
    IID_PPV_ARGS(m_RootSignature.GetAddressOf())));

    m_FenceEvent = CreateEvent(nullptr, false, false, nullptr);
    if(m_FenceEvent == nullptr)
    {
        ThrowIfFiled(HRESULT_FROM_WIN32(GetLastError()));
    }
    ThrowIfFiled(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
    IID_PPV_ARGS(m_Fence.GetAddressOf())));
    m_BackCurrentFrame = m_SwapChain->GetCurrentBackBufferIndex();
    m_FenceValue[m_BackCurrentFrame]++;
    ComPtr<ID3DBlob> vShader;
    ComPtr<ID3DBlob> pShader;
    UINT shaderFlag = 0;
    #if defined(_DEBUG) || defined(DEBUG)
        shaderFlag |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    #endif
    ThrowIfFiled(D3DCompileFromFile(L"src/HelloBandles/shader/shader.hlsl", nullptr, nullptr, "VSmain", "vs_5_0", 
    shaderFlag, 0, vShader.GetAddressOf(), nullptr));
    ThrowIfFiled(D3DCompileFromFile(L"src/HelloBandles/shader/shader.hlsl",
    nullptr, nullptr, "PSmain", "ps_5_0", shaderFlag, 0, pShader.GetAddressOf(), nullptr));

    D3D12_INPUT_ELEMENT_DESC inputLayout[]{
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphysicalPipelineStateDesc{};
    graphysicalPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    graphysicalPipelineStateDesc.pRootSignature = m_RootSignature.Get();
    graphysicalPipelineStateDesc.VS = CD3DX12_SHADER_BYTECODE(vShader.Get());
    graphysicalPipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(pShader.Get());
    graphysicalPipelineStateDesc.SampleMask = UINT_MAX;
    graphysicalPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    graphysicalPipelineStateDesc.InputLayout = {inputLayout, _countof(inputLayout)};
    graphysicalPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    graphysicalPipelineStateDesc.NumRenderTargets = 1;
    graphysicalPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    graphysicalPipelineStateDesc.SampleDesc.Count = 1;
    graphysicalPipelineStateDesc.SampleDesc.Quality = 0;
    graphysicalPipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    

    ThrowIfFiled(m_Device->CreateGraphicsPipelineState(&graphysicalPipelineStateDesc, 
    IID_PPV_ARGS(m_PipelineState.GetAddressOf())));
    Vertex vertices[]{
        {{-1.0f, -1.0f, .0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{-1.0f, +1.0f, .0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{+1.0f, +1.0f, .0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
        {{+1.0f, -1.0f, .0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{-1.0f, -1.0f, +1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{-1.0f, +1.0f, +1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
        {{+1.0f, +1.0f, +1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{+1.0f, -1.0f, +1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}}
    };
    UINT verticesSize = sizeof(vertices);
    
    ThrowIfFiled(m_Device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(verticesSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr, IID_PPV_ARGS(vertexBuffer.GetAddressOf())
    ));
    UINT8* vertexBufferPtr;
    CD3DX12_RANGE range(0, 0);
    ThrowIfFiled(vertexBuffer->Map(0, &range, reinterpret_cast<void**>(&vertexBufferPtr)));
    memcpy(vertexBufferPtr, vertices, verticesSize);
    vertexBuffer->Unmap(0, nullptr);
    m_VertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = verticesSize;
    m_VertexBufferView.StrideInBytes = sizeof(Vertex);

    ThrowIfFiled(m_CommandList->Close());
    
    ThrowIfFiled(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, m_BundleAllocator.Get(), m_PipelineState.Get(),
        IID_PPV_ARGS(m_Bundle.GetAddressOf())));
    {
        m_Bundle->SetGraphicsRootSignature(m_RootSignature.Get());
        m_Bundle->IASetVertexBuffers(0, 1, &m_VertexBufferView);
        m_Bundle->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ThrowIfFiled(m_Bundle->Close());
    }
    //create constant
// 创建视图矩阵
    DirectX::XMVECTOR eyePos = DirectX::XMVectorSet(5.0f, 5.0f, -5.0f, 1.0f);
    DirectX::XMVECTOR targetPos = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    DirectX::XMVECTOR upVector = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    //upVector = DirectX::XMVector3Cross(DirectX::XMVectorSet(0.0f, -2.0f, 5.0f, 0.0f), DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eyePos, targetPos, upVector);

    // 创建投影矩阵
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        0.25f * 3.1416f,
        static_cast<float>(m_Width) / static_cast<float>(m_Height),
        0.1f,
        100.0f
    );

    // 计算视图投影矩阵，注意矩阵乘法顺序
    DirectX::XMMATRIX viewProjectionMatrix = viewMatrix * projectionMatrix;
    // 将最终的视图投影矩阵存储到常量缓冲区结构体中
    DirectX::XMStoreFloat4x4(&m_Constant.proj, viewProjectionMatrix);
    //DirectX::XMStoreFloat4x4(&m_Constant.proj, DirectX::XMMatrixIdentity());
    m_Constant.move = { 0.0f, 0.0f, 0.0f ,0.0f };
    {
        const size_t m_ConstantSize = (sizeof(m_Constant) + 255) & (~255);
        ThrowIfFiled(m_Device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(m_ConstantSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_ConstantBuffer.GetAddressOf())
        ));

        constantBufferViewDesc.BufferLocation = m_ConstantBuffer->GetGPUVirtualAddress();
        constantBufferViewDesc.SizeInBytes = m_ConstantSize;
        m_Device->CreateConstantBufferView(&constantBufferViewDesc, m_SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

        UINT8* constantPtr;
        CD3DX12_RANGE readRange(0, 0);
        ThrowIfFiled(m_ConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&constantPtr)));
        memcpy(constantPtr, &m_Constant, m_ConstantSize);
        m_ConstantBuffer->Unmap(0, nullptr);
    }
    std::array<std::uint16_t, 36> indices =
    {
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
    const size_t indicesBufferCount = sizeof(indices);
    ThrowIfFiled(m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(indicesBufferCount), D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr, IID_PPV_ARGS(m_IndexBuffer.GetAddressOf())));
    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_IndexBufferView.SizeInBytes = indicesBufferCount;

    UINT8* indexPtr;
    CD3DX12_RANGE indexRange(0, 0);
    ThrowIfFiled(m_IndexBuffer->Map(0, &indexRange, reinterpret_cast<void**>(&indexPtr)));
    memcpy(indexPtr, &indices[0], indicesBufferCount);
    m_IndexBuffer->Unmap(0, nullptr);

    D3D12_RESOURCE_DESC depthDesc{};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Alignment = 0;
    depthDesc.Width = m_Width;
    depthDesc.Height = m_Height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    CD3DX12_CLEAR_VALUE depthClearOpt(DXGI_FORMAT_D24_UNORM_S8_UINT, 1.0f, 0);
    ThrowIfFiled(m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE, &depthDesc, D3D12_RESOURCE_STATE_COMMON,
        &depthClearOpt, IID_PPV_ARGS(m_DepthBuffer.GetAddressOf())));

    

    m_Device->CreateDepthStencilView(m_DepthBuffer.Get(),nullptr, m_dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

}

void Bandles::LoadPipeline()
{
    UINT m_FactoryFlags = 0;
#if defined(_DEBUG) || defined(DEBUG)
    ComPtr<ID3D12Debug> debug;
    ThrowIfFiled(D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf())));
    debug->EnableDebugLayer();
    m_FactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    ComPtr<IDXGIFactory7> m_Factory;
    ThrowIfFiled(CreateDXGIFactory2(m_FactoryFlags, IID_PPV_ARGS(m_Factory.GetAddressOf())));
    if(m_UseWarp)
    {
        m_Factory->EnumWarpAdapter(IID_PPV_ARGS(m_Adapter.GetAddressOf()));
    }
    else{
        GetAdapter(m_Factory, m_Adapter.GetAddressOf());
    }
    ThrowIfFiled(D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.GetAddressOf())));
    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.NodeMask = 0;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ThrowIfFiled(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf())));
    ThrowIfFiled(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, 
    IID_PPV_ARGS(m_CommandAllocator.GetAddressOf())));

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc1{};
    swapChainDesc1.Width = m_Width;
    swapChainDesc1.Height = m_Height;
    swapChainDesc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc1.SampleDesc.Count = 1;
    swapChainDesc1.SampleDesc.Quality = 0;
    swapChainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc1.BufferCount = m_FrameCount;
    swapChainDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFiled(m_Factory->CreateSwapChainForHwnd(m_CommandQueue.Get(),
    BandlesApp::getInstance(), &swapChainDesc1, nullptr, nullptr, swapChain.GetAddressOf()
    ));

    ThrowIfFiled(swapChain.As(&m_SwapChain));
    ThrowIfFiled(m_Device->CreateDescriptorHeap(
        &rtvHeapDesc, IID_PPV_ARGS(m_RtvDescriptorHeap.GetAddressOf())
    ));

    D3D12_DESCRIPTOR_HEAP_DESC srvDescriptorHeapDesc{};
    srvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvDescriptorHeapDesc.NumDescriptors = 1;
    srvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    srvDescriptorHeapDesc.NodeMask = 0;

    ThrowIfFiled(m_Device->CreateDescriptorHeap(&srvDescriptorHeapDesc,
    IID_PPV_ARGS(m_SrvDescriptorHeap.GetAddressOf())));

    m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_SrvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    for(UINT n = 0; n < m_FrameCount; n++)
    {
        ThrowIfFiled(m_SwapChain->GetBuffer(n, IID_PPV_ARGS(m_RtvBuffer[n].GetAddressOf())));
        m_Device->CreateRenderTargetView(
            m_RtvBuffer[n].Get(), nullptr, rtvHandle
        );
        rtvHandle.Offset(1, m_RtvDescriptorSize);
    }
    D3D12_DESCRIPTOR_HEAP_DESC depthDesc{};
    depthDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    depthDesc.NodeMask = 0;
    depthDesc.NumDescriptors = 1;
    depthDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    ThrowIfFiled(m_Device->CreateDescriptorHeap(&depthDesc, IID_PPV_ARGS(m_dsDescriptorHeap.GetAddressOf())));
    ThrowIfFiled(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(m_BundleAllocator.GetAddressOf())));
}

void Bandles::GetAdapter(ComPtr<IDXGIFactory7>& pFactory, IDXGIAdapter1 **adapter)
{
    *adapter = nullptr;
    ComPtr<IDXGIAdapter1> pAdapter;
    if(SUCCEEDED(pFactory->QueryInterface(_uuidof(IDXGIFactory6), nullptr)))
    {
        for(UINT adapterIndex = 0;
        SUCCEEDED(pFactory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, 
        IID_PPV_ARGS(pAdapter.GetAddressOf()))); adapterIndex++)
        {
            DXGI_ADAPTER_DESC1 desc{};
            pAdapter->GetDesc1(&desc);
            if(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }

            if(SUCCEEDED(D3D12CreateDevice(pAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }
    if(pAdapter == nullptr)
    {
        for(UINT adapterIndex = 0;
        SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, pAdapter.GetAddressOf()));
        adapterIndex++)
        {
            DXGI_ADAPTER_DESC1 adapterDesc1{};
            pAdapter->GetDesc1(&adapterDesc1);

            if(adapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }
            if(SUCCEEDED(D3D12CreateDevice(pAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }
    *adapter = pAdapter.Detach();
}

void Bandles::waitForFinish()
{
    const int fence = m_FenceValue[m_BackCurrentFrame];
    m_BackCurrentFrame = m_SwapChain->GetCurrentBackBufferIndex();
    ThrowIfFiled(m_CommandQueue->Signal(m_Fence.Get(), fence));
    if(m_Fence->GetCompletedValue() < m_FenceValue[m_BackCurrentFrame])
    {
        ThrowIfFiled(m_Fence->SetEventOnCompletion(m_FenceValue[m_BackCurrentFrame], m_FenceEvent));
        WaitForSingleObject(m_FenceEvent, INFINITE);
    }
    m_FenceValue[m_BackCurrentFrame] = fence + 1;
}

void Bandles::fillOutCommandList()
{
    ThrowIfFiled(m_CommandAllocator->Reset());
    ThrowIfFiled(m_CommandList->Reset(m_CommandAllocator.Get(), m_PipelineState.Get()));

    m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_BackCurrentFrame, m_RtvDescriptorSize);
    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RtvBuffer[m_BackCurrentFrame].Get(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    m_CommandList->OMSetRenderTargets(1, &rtvHandle, true, &m_dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    m_CommandList->RSSetViewports(1, &m_Viewport);
    m_CommandList->RSSetScissorRects(1, &m_Sissor);
    float clearColor[]{0.0f, 0.3f, 0.2f, 1.0f};
    m_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_CommandList->ClearDepthStencilView(m_dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        1.0f, 0, 0, nullptr);
    ID3D12DescriptorHeap* descriptorHeaps[]{ m_SrvDescriptorHeap.Get()};
    m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    m_CommandList->SetGraphicsRootDescriptorTable(0, m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
    m_CommandList->ExecuteBundle(m_Bundle.Get());
    m_CommandList->IASetIndexBuffer(&m_IndexBufferView);
    m_CommandList->DrawIndexedInstanced(12, 1, 0, 0, 0);
    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RtvBuffer[m_BackCurrentFrame].Get(),
    D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFiled(m_CommandList->Close());
}
#endif
