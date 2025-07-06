#include "D3DUtil.h"
#include "core/d3dx12.h"
namespace d3dUtil
{
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* cmdList,
    const void* initData,
    UINT64 byteSize,
    Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer
)
    {
        using Microsoft::WRL::ComPtr;
        ComPtr<ID3D12Resource> defaultBuffer;
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(defaultBuffer.GetAddressOf())
        ));

        ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(uploadBuffer.GetAddressOf())
        ));

        D3D12_SUBRESOURCE_DATA subResourceData;
        subResourceData.pData = initData;
        subResourceData.RowPitch = byteSize;
        subResourceData.SlicePitch = byteSize;

        cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            defaultBuffer.Get(), D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_COPY_DEST
        ));
        UpdateSubresources(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);

        cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            defaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_COMMON
        ));
        return defaultBuffer;
    }
    UINT CalcConstBufferByteSize(UINT byteSize)
    {
        return (byteSize + 255) & ~255;
    }
}