#pragma once
#include "Shader.h"
#include "Mesh.h"

class MultiMesh
{
public:
    MultiMesh(int width, int height)
        : m_Width(width), m_Height(height)
    {
        m_ViewPort = D3D12_VIEWPORT{0, 0, static_cast<float>(m_Width), static_cast<float>(m_Height)};
        m_Scissor = D3D12_RECT{0, 0, m_Width, m_Height};
    }

    void OnInint();
    void OnUpdate();
    void OnRender();
    void OnDestroy();

    int getWidth() { return m_Width; }
    int getHeight() { return m_Height; }

private:
    void initPipeline();
    void initAsserts();
    void createDevice(ComPtr<IDXGIFactory7> factory);
    void createCommandQueue();
    void createCommandAllocator();
    void createCommandList();
    void createFence();
    DXGI_SAMPLE_DESC createSampleDesc(const DXGI_FORMAT& format);
    void createSwapChain(const ComPtr<IDXGIFactory7>& factory);
    void createDescriptorHeap();
    void createRTV();
    void createDSV();
    
    void populateCommanddList();
    void waitForFinish();
    bool checkTearingSupport(ComPtr<IDXGIFactory7>& factory);
    int m_Width;
    int m_Height;
    static constexpr int m_RenderTartgetBufferCount = 2;
    int m_RenderTargetBufferIndex = 0;
    int m_RTVDescriptorSize;

    D3D12_VIEWPORT m_ViewPort;
    D3D12_RECT m_Scissor;
    ComPtr<IDXGIAdapter4> m_Adapter;
    ComPtr<ID3D12Device> m_Device;
    ComPtr<ID3D12CommandQueue> m_CommandQueue;
    ComPtr<ID3D12Fence> m_Fence;
    HANDLE m_FenceEvent;
    ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
    ComPtr<ID3D12DescriptorHeap> m_DSVDescriptorHeap;
    ComPtr<ID3D12DescriptorHeap> m_CBV_SRV_UAVDescriptorHeap;
    ComPtr<IDXGISwapChain4> m_SwapChain;
    ComPtr<ID3D12Resource> m_RenderTargetBuffer[m_RenderTartgetBufferCount];
    UINT m_RenderTargetValue[m_RenderTartgetBufferCount];
    ComPtr<ID3D12Resource> m_DepthStencilBuffer;
    ComPtr<ID3D12CommandAllocator> m_MainAllocator;
    ComPtr<ID3D12GraphicsCommandList> m_MainCommandList;
    Shader m_Shader;
    Scene m_Scene;
};
