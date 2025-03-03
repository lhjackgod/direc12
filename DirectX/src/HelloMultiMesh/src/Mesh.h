#pragma once
#include "Shader.h"

using namespace DirectX;
namespace MYMESH
{
    struct Vertex
    {
        XMFLOAT3 aPos;
        //XMFLOAT2 aUV;
        XMFLOAT4 aColor;
    };
}

class Mesh
{
public:
    Mesh()
    {
        inputElements = new D3D12_INPUT_ELEMENT_DESC[2]
        {
            {
                "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
                0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                0
            },
            {
                "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
                0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                0
            }
        };
        inputElementCount = 2;
    }
    void OnInit(MYMESH::Vertex* vertices, UINT vertexCount, const ComPtr<ID3D12Device>& device);
    static D3D12_INPUT_ELEMENT_DESC* getInputElementDesc();
    static UINT getInputElementCount();
    void render(ComPtr<ID3D12GraphicsCommandList>& cmdList,
        Shader& shader);
private:
    MYMESH::Vertex* m_Vertices;
    ComPtr<ID3D12Resource> m_VertexBuffer;
    UINT m_VertexCount;
    static D3D12_INPUT_ELEMENT_DESC* inputElements;
    static UINT inputElementCount;
private:
    D3D12_VERTEX_BUFFER_VIEW getVertexBufferView();
};
