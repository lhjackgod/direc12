#include "pch.h"
#include "MultiMesh.h"

#include <queue>

#include "WinApp.h"


void MultiMesh::OnInint()
{
    initPipeline();
    initAsserts();
}

void MultiMesh::OnUpdate()
{
}

void MultiMesh::OnRender()
{
    waitForFinish();
    populateCommanddList();
    ID3D12CommandList* commandLists[]{m_MainCommandList.Get()};
    m_CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
    m_SwapChain->Present(1, 0);
    waitForFinish();
}

void MultiMesh::OnDestroy()
{
    free(m_FenceEvent);
}

bool MultiMesh::checkTearingSupport(ComPtr<IDXGIFactory7>& factory)
{
    bool ret = false;
    factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &ret, sizeof(ret));
    return ret;
}

void MultiMesh::initPipeline()
{
    ComPtr<ID3D12Debug> debugController;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf())));
    debugController->EnableDebugLayer();
    ComPtr<IDXGIFactory7> factory;
    ThrowIfFailed(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(factory.GetAddressOf())));
    if (checkTearingSupport(factory))
    {
        throw std::runtime_error("Tearing support is not supported");
    }
    createDevice(factory);
    createCommandQueue();
    createCommandAllocator();
    createCommandList();
    createFence();
    createDescriptorHeap();
    createSwapChain(factory);
    createRTV();
    createDSV();
}

void MultiMesh::initAsserts()
{
    DIRECTX::MeshData Sphere = GenerateSphereMesh(1.0f, 32, 16);
    m_Scene.addMesh(Sphere);
    MYMESH::Vertex v1 = {
        {-1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}
    };
    MYMESH::Vertex v2 = {
        {-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}
    };
    MYMESH::Vertex v3 = {
        {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}
    };
    MYMESH::Vertex v4 = {
        {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}
    };
    MYMESH::Vertex v5 = {
        {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}
    };
    MYMESH::Vertex v6 = {
        {1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}
    };
    auto triangle1 = new MYMESH::Vertex[3]{v1, v2, v3};
    auto triangle2 = new MYMESH::Vertex[3]{v4, v5, v6};
    m_Shader.OnInit(L"src/shader/shader.hlsl", L"src/shader/shader.hlsl", DIRECTX::Vertex::getInputElementDesc(),
                    DIRECTX::Vertex::getInputSize(), nullptr, 0, 0, nullptr, m_Device
    );
}

void MultiMesh::createDevice(ComPtr<IDXGIFactory7> factory)
{
    ComPtr<IDXGIAdapter1> pAdapter;
    for (UINT adapterIndex = 0;
         factory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                                             IID_PPV_ARGS(pAdapter.GetAddressOf())); adapterIndex++)
    {
        DXGI_ADAPTER_DESC1 adapterDesc{};
        pAdapter->GetDesc1(&adapterDesc);
        if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }
        if (SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
        {
            break;
        }
    }
    if (pAdapter == nullptr)
    {
        for (UINT adapterIndex = 0; factory->EnumAdapters1(adapterIndex, pAdapter.GetAddressOf()); adapterIndex++)
        {
            DXGI_ADAPTER_DESC1 desc{};
            pAdapter->GetDesc1(&desc);
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }
            if (SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }
    if (pAdapter == nullptr)
    {
        factory->EnumWarpAdapter(IID_PPV_ARGS(pAdapter.GetAddressOf()));
    }
    pAdapter.As(&m_Adapter);
    ThrowIfFailed(D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(m_Device.GetAddressOf())));
}

void MultiMesh::createCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf())));
}

void MultiMesh::createCommandAllocator()
{
    ThrowIfFailed(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                   IID_PPV_ARGS(m_MainAllocator.GetAddressOf())));
}

void MultiMesh::createCommandList()
{
    ThrowIfFailed(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                              m_MainAllocator.Get(), nullptr,
                                              IID_PPV_ARGS(m_MainCommandList.GetAddressOf())));
    ThrowIfFailed(m_MainCommandList->Close());
}

void MultiMesh::createFence()
{
    ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf())));
    m_FenceEvent = CreateEvent(nullptr, false, false, nullptr);
    if (m_FenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }
}

DXGI_SAMPLE_DESC MultiMesh::createSampleDesc(const DXGI_FORMAT& format)
{
    DXGI_SAMPLE_DESC sampleDesc{};
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels{};
    msQualityLevels.SampleCount = 4;
    msQualityLevels.Format = format;
    if (m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
                                      &msQualityLevels, sizeof(msQualityLevels)))
    {
        sampleDesc.Count = 4;
        sampleDesc.Quality = msQualityLevels.NumQualityLevels - 1;
    }
    else
    {
        sampleDesc.Count = 1;
        sampleDesc.Quality = 0;
    }
    return sampleDesc;
}

void MultiMesh::createSwapChain(const ComPtr<IDXGIFactory7>& factory)
{
    ComPtr<IDXGISwapChain1> pSwapChain;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = m_Width;
    swapChainDesc.Height = m_Height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SampleDesc = createSampleDesc(swapChainDesc.Format);
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = m_RenderTartgetBufferCount;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    if (WinApp::getWindowHandle() == nullptr)
    {
        std::cerr << "jack" << std::endl;
    }
    ThrowIfFailed(factory->CreateSwapChainForHwnd(m_CommandQueue.Get(), WinApp::getWindowHandle(), &swapChainDesc,
                                                  nullptr, nullptr,
                                                  pSwapChain.GetAddressOf()));
    ThrowIfFailed(pSwapChain.As(&m_SwapChain));
    m_RenderTargetBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
    m_RenderTargetValue[m_RenderTargetBufferIndex] = 1;
}

void MultiMesh::createDescriptorHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.NumDescriptors = m_RenderTartgetBufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_RTVDescriptorHeap.GetAddressOf())));

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_DSVDescriptorHeap.GetAddressOf())));

    D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc{};
    cbvSrvUavHeapDesc.NumDescriptors = 1;
    cbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(m_Device->CreateDescriptorHeap(&cbvSrvUavHeapDesc,
                                                 IID_PPV_ARGS(m_CBV_SRV_UAVDescriptorHeap.GetAddressOf())));
    
}

void MultiMesh::createRTV()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    m_RTVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    for (int i = 0; i < m_RenderTartgetBufferCount; i++)
    {
        ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_RenderTargetBuffer[i].GetAddressOf())));
        m_Device->CreateRenderTargetView(m_RenderTargetBuffer[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_RTVDescriptorSize);
    }
}

void MultiMesh::createDSV()
{
    D3D12_RESOURCE_DESC depthStencilDesc{};
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = m_Width;
    depthStencilDesc.Height = m_Height;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc = createSampleDesc(depthStencilDesc.Format);
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = depthStencilDesc.Format;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    ThrowIfFailed(m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                    D3D12_HEAP_FLAG_NONE, &depthStencilDesc,
                                                    D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue,
                                                    IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf())));

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = depthStencilDesc.Format;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &dsvDesc,
                                     m_DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void MultiMesh::populateCommanddList()
{
    ThrowIfFailed(m_MainAllocator->Reset());
    ThrowIfFailed(m_MainCommandList->Reset(m_MainAllocator.Get(), m_Shader.getPipelineSate().Get()));
    m_MainCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                           m_RenderTargetBuffer[m_RenderTargetBufferIndex].Get(),
                                           D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    CD3DX12_CPU_DESCRIPTOR_HANDLE renderTarget(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                                               m_RenderTargetBufferIndex, m_RTVDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle(m_DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    m_MainCommandList->OMSetRenderTargets(1, &renderTarget, false, &depthHandle);
    m_MainCommandList->RSSetViewports(1, &m_ViewPort);
    m_MainCommandList->RSSetScissorRects(1, &m_Scissor);
    float clearColor[4] = {0.0f, 0.3f, 0.2f, 1.0f};
    m_MainCommandList->ClearRenderTargetView(renderTarget, clearColor, 0, nullptr);
    m_MainCommandList->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    reinterpret_cast<ID3D12GraphicsCommandList5*>(m_MainCommandList.Get())->RSSetShadingRate(D3D12_SHADING_RATE_4X4, nullptr);
    
    m_MainCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                                           m_RenderTargetBuffer[m_RenderTargetBufferIndex].Get(),
                                           D3D12_RESOURCE_STATE_RENDER_TARGET,
                                           D3D12_RESOURCE_STATE_PRESENT));
    ThrowIfFailed(m_MainCommandList->Close());
}

void MultiMesh::waitForFinish()
{
    int fenceValue = m_RenderTargetValue[m_RenderTargetBufferIndex];
    m_RenderTargetBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
    m_CommandQueue->Signal(m_Fence.Get(), fenceValue);
    if (m_Fence->GetCompletedValue() < m_RenderTargetValue[m_RenderTargetBufferIndex])
    {
        ThrowIfFailed(m_Fence->SetEventOnCompletion(m_RenderTargetValue[m_RenderTargetBufferIndex],
                                                    m_FenceEvent));
        WaitForSingleObject(m_FenceEvent, INFINITE);
    }
    m_RenderTargetValue[m_RenderTargetBufferIndex] = fenceValue + 1;
}
