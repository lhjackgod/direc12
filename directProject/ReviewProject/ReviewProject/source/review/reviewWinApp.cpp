#include "D3DUtil.h"
#include "review/reviewWinApp.h"

#include "d3dx12.h"
#include <DirectXMath.h>
#include <DirectXColors.h>
bool ReviewApp::Initialize()
{
    if (!D3DApp::Initialize())
    {
        return false;
    }
    return true;
}

void ReviewApp::Update(const GameTimer& gt)
{
    
}

void ReviewApp::Draw(const GameTimer& gt)
{
    ThrowIfFailed(m_CommandAllocator->Reset());

    ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(),
        nullptr));

    D3D12_RESOURCE_BARRIER rtvResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_SwapChainBuffers[m_CurrentBackBufferIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_CommandList->ResourceBarrier(1, &rtvResourceBarrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = CurrentBackBufferHandle();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DepthStencilView();
    m_CommandList->OMSetRenderTargets(1, &rtvHandle, true, &dsvHandle);
    m_CommandList->RSSetViewports(1, &m_ViewPort);
    m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

    float clearColor[4] = {1.0f, 1.0f, 0.0f, 1.0f};
    m_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_CommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        1.0f, 0, 0, nullptr);

    D3D12_RESOURCE_BARRIER rtvResourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
        m_SwapChainBuffers[m_CurrentBackBufferIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    m_CommandList->ResourceBarrier(1, &rtvResourceBarrier2);
    ThrowIfFailed(m_CommandList->Close());
    ID3D12CommandList* commandLists[] = {m_CommandList.Get()};
    m_CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
    ThrowIfFailed(m_SwapChain->Present(0, 0));
    m_CurrentBackBufferIndex = (m_CurrentBackBufferIndex + 1) % SwapChainBufferCount;
    FlushCommandQueue();
}

void readBookTest()
{
    ComPtr<ID3D12GraphicsCommandList> commandlist;
    commandlist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    // ... 通过线列表来绘制对象 ... //
    commandlist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // ... !!!(最常用)通过三角列表绘制对象 ... //
    commandlist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    // ...通过三角形带来绘制对象 ...//
    commandlist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
    commandlist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);
    D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
    D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
    using namespace DirectX;
    XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f * XM_PI, 800.0f/600.0f,1.0f, 100.0f);
    struct Vertex1
    {
        XMFLOAT3 Pos;
        XMFLOAT4 Color;
    };
    struct Vertex2
    {
        XMFLOAT3 pos;
        XMFLOAT3 Normal;
        XMFLOAT2 Tex0;
        XMFLOAT2 Tex1;
    };
    Vertex1 vertices[] = {
        {XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White)},
           {XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(Colors::Black)},
        {XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(Colors::Red)},
        {XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green)},
    };
    const UINT64 vbByteSize = 4 * sizeof(Vertex1);
    ComPtr<ID3D12Resource> VertexBuffer;
    ComPtr<ID3D12Resource> VertexBufferUpload;
    VertexBuffer = d3dUtil::CreateDefaultBuffer(device, cmdList, vertices, vbByteSize, VertexBufferUpload);
    VertexBuffer->GetGPUVirtualAddress();
    ComPtr<ID3D12GraphicsCommandList> cmdList;
    D3D12_VERTEX_BUFFER_VIEW vbv;
    vbv.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
    vbv.SizeInBytes = sizeof(4 * sizeof(Vertex1));
    vbv.StrideInBytes = sizeof(Vertex1);

    D3D12_VERTEX_BUFFER_VIEW vertexBuffers[1] = {vbv};
    cmdList->IASetVertexBuffers(0, _countof(vertexBuffers), vertexBuffers);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->DrawInstanced(4, 1, 0, 0);
    std::uint16_t indices[] = {
        0, 1, 2,
        0, 2, 3,

        4, 5, 6,
        4, 7, 6,

        4, 5, 1,
        4, 1, 0,

        3, 2, 6,
        3, 6, 7,

        1, 5, 6,
        1, 6, 2,

        4, 0, 3,
        4, 3, 7
    };
    const UINT idByteSize = 36 * sizeof(std::uint16_t);
    ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;
    ComPtr<ID3D12Resource> IndexBufferUpload = nullptr;
    IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device, cmdList, indices, idByteSize, IndexBufferUpload);

    D3D12_INDEX_BUFFER_VIEW ibv;
    ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
    ibv.SizeInBytes = idByteSize;
    ibv.Format = DXGI_FORMAT_R16_UINT;

    cmdList->IASetIndexBuffer(&ibv);
    cmdList->DrawIndexedInstanced();
    struct ObjectConstants
    {
        XMFLOAT4X4 WorldViewProj;
    };
    UINT mElementByteSize = d3dUtil::CalcConstBufferByteSize(sizeof(ObjectConstants));
    ComPtr<ID3D12Resource> mUploadBuffer;
    ComPtr<ID3D12Device> pDevice;
    pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(NumElements * mElementByteSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(mUploadBuffer.GetAddressOf()));
    
}
