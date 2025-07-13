#include "D3DUtil.h"
#include "BoxApp.h"

BoxApp::BoxApp(HINSTANCE hInstance)
    :D3DApp(hInstance)
{
    
}

BoxApp::~BoxApp()
{
}

void BoxApp::OnResize()
{
    D3DApp::OnResize();

    DirectX::XMMATRIX p = DirectX::XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    DirectX::XMStoreFloat4x4(&m_Proj, p);
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
    D3DApp::OnMouseMove(btnState, x, y);
}

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
    D3DApp::OnMouseDown(btnState, x, y);
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
    D3DApp::OnMouseUp(btnState, x, y);
}

void BoxApp::BuildDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc{};
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.NodeMask = 0;
    
    ThrowIfFailed(m_Device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(m_ConstBufferHeap.GetAddressOf())));
}

void BoxApp::BuildConstBuffer()
{
    m_ObjectCB = std::make_unique<UploadBuffer<ObjectConsts>>(m_Device.Get(), 1, true);

    UINT objCBByteSize = d3dUtil::CalcConstBufferByteSize(sizeof(ObjectConsts));

    D3D12_GPU_VIRTUAL_ADDRESS cbAdress = static_cast<ID3D12Resource*>(*m_ObjectCB)->GetGPUVirtualAddress();

    // 偏移到常量缓冲区中第i个物体所对应的常量数据
    //这里取i = 0
    int boxCBufferIndex = 0;
    cbAdress += boxCBufferIndex * objCBByteSize;
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
    cbvDesc.BufferLocation = cbAdress;
    cbvDesc.SizeInBytes = objCBByteSize;
    
    m_Device->CreateConstantBufferView(&cbvDesc, m_ConstBufferHeap->GetCPUDescriptorHandleForHeapStart());
}

void BoxApp::BuildRootSignature()
{
    
}

void BoxApp::BuildShadersAndInputLayout()
{
    
}

void BoxApp::BuildBoxGeometry()
{
    
}

void BoxApp::BuildPSO()
{
    
}

bool BoxApp::Initialize()
{
    if (!D3DApp::Initialize())
    {
        return false;
    }
    ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), m_PSO.Get()));
    BuildDescriptorHeaps();
    BuildConstBuffer();
    BuildRootSignature();
    BuildShadersAndInputLayout();
    BuildBoxGeometry();
    BuildPSO();

    ThrowIfFailed(m_CommandList->Close());
    ID3D12CommandList* cmdLists[] = { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
    FlushCommandQueue();
    
    return true;
}

void BoxApp::Update(const GameTimer& gt)
{
    
}

void BoxApp::Draw(const GameTimer& gt)
{
    
}
