#include "pch.h"
#include "Mesh.h"

D3D12_INPUT_ELEMENT_DESC* Mesh::inputElements = nullptr;
UINT Mesh::inputElementCount = 0;
void Mesh::OnInit(MYMESH::Vertex* vertices, UINT vertexCount,
    const ComPtr<ID3D12Device>& device)
{
    UINT vertexCountByte = sizeof(MYMESH::Vertex) * vertexCount;
    m_Vertices = vertices;
    m_VertexCount = vertexCount;
    ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexCountByte),
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_VertexBuffer.GetAddressOf())));
    UINT8* pData = nullptr;
    D3D12_RANGE range{0, 0,};
    ThrowIfFailed(m_VertexBuffer->Map(0, &range, reinterpret_cast<void**>(&pData)));
    memcpy(pData, m_Vertices, vertexCountByte);
    m_VertexBuffer->Unmap(0, nullptr);
}

D3D12_INPUT_ELEMENT_DESC* Mesh::getInputElementDesc()
{
    return inputElements;
}

UINT Mesh::getInputElementCount()
{
    return inputElementCount;
}

void Mesh::render(ComPtr<ID3D12GraphicsCommandList>& cmdList, Shader& shader)
{
    cmdList->SetGraphicsRootSignature(shader.getRootSignature().Get());
    cmdList->SetPipelineState(shader.getPipelineSate().Get());
    cmdList->IASetVertexBuffers(0, 1, &getVertexBufferView());
    cmdList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->DrawInstanced(m_VertexCount, 1, 0, 0);
}

D3D12_VERTEX_BUFFER_VIEW Mesh::getVertexBufferView()
{
    D3D12_VERTEX_BUFFER_VIEW vetexBufferView{};
    vetexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    vetexBufferView.StrideInBytes = sizeof(MYMESH::Vertex);
    vetexBufferView.SizeInBytes = m_VertexCount * sizeof(MYMESH::Vertex);
    return vetexBufferView;
}
