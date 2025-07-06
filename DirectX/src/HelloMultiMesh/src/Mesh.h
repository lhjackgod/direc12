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

namespace DIRECTX
{
    struct Vertex
    {
        Vertex() = default;
        Vertex(const XMFLOAT3& p,
            const XMFLOAT3& n,
            const XMFLOAT3& t,
            const XMFLOAT2& uv)
                :Position(p), Normal(n), Tangent(t), Texc(uv) {}
        Vertex(
            float px, float py, float pz,
            float nx, float ny, float nz,
            float tx, float ty, float tz,
            float u, float v) :
        Position{px, py, pz}, Normal{nx, ny, nz}, Tangent{tx, ty, tz},
        Texc{u, v} {}
        XMFLOAT3 Position;
        XMFLOAT3 Normal;
        XMFLOAT3 Tangent;
        XMFLOAT2 Texc;
        static UINT getInputSize()
        {
            return 4;
        }
        static D3D12_INPUT_ELEMENT_DESC* getInputElementDesc()
        {
            static D3D12_INPUT_ELEMENT_DESC inputElementDesc[]
            {
              {
                  "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
              },
                {
                    "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
                    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
                },
                {
                    "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24,
                    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
                },
                {
                    "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36,
                    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
                }
            };
            return inputElementDesc;
        }
    };
    struct MeshData
    {
        std::vector<Vertex> vertices;
        std::vector<UINT> indices;
    };
}

class Scene
{
public:
    Scene() = default;
    void addMesh(const DIRECTX::MeshData& mesh);
    void finishScene(const ComPtr<ID3D12Device>& device);
private:
    std::vector<DIRECTX::Vertex> m_Data;
    std::vector<UINT> m_Indices;
    ComPtr<ID3D12Resource> m_VertexBuffer;
    ComPtr<ID3D12Resource> m_IndexBuffer;
private:
    void createIndexBuffer(const ComPtr<ID3D12Device>& device);
    void createVertexBuffer(const ComPtr<ID3D12Device>& device);
    D3D12_VERTEX_BUFFER_VIEW getVertexBufferView(const ComPtr<ID3D12Device>& device);
    D3D12_INDEX_BUFFER_VIEW getIndexBufferView(const ComPtr<ID3D12Device>& device);
    void render(ComPtr<ID3D12GraphicsCommandList>& cmdList, Shader& shader);
};

/*class Mesh
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
    D3D12_VERTEX_BUFFER_VIEW getVertexBufferView();
};*/

// 生成球体顶点和索引数据的函数
inline DIRECTX::MeshData GenerateSphereMesh(float radius, UINT sliceCount, UINT stackCount)
{
    using namespace DIRECTX;
    MeshData meshData;

    // 顶部顶点
    Vertex topVertex;
    topVertex.Position = DirectX::XMFLOAT3(0.0f, radius, 0.0f);
    topVertex.Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
    topVertex.Tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
    topVertex.Texc = DirectX::XMFLOAT2(0.0f, 0.0f);
    meshData.vertices.push_back(topVertex);

    // 中间层顶点
    float phiStep = DirectX::XM_2PI / sliceCount;
    float thetaStep = DirectX::XM_PI / stackCount;

    for (UINT i = 1; i < stackCount; ++i)
    {
        float theta = i * thetaStep;
        for (UINT j = 0; j <= sliceCount; ++j)
        {
            float phi = j * phiStep;

            Vertex vertex;
            vertex.Position.x = radius * sinf(theta) * cosf(phi);
            vertex.Position.y = radius * cosf(theta);
            vertex.Position.z = radius * sinf(theta) * sinf(phi);

            // 法线是从球心指向顶点的向量
            vertex.Normal.x = vertex.Position.x / radius;
            vertex.Normal.y = vertex.Position.y / radius;
            vertex.Normal.z = vertex.Position.z / radius;

            // 切线
            vertex.Tangent.x = -radius * sinf(theta) * sinf(phi);
            vertex.Tangent.y = 0.0f;
            vertex.Tangent.z = radius * sinf(theta) * cosf(phi);
            DirectX::XMVECTOR tangent = DirectX::XMLoadFloat3(&vertex.Tangent);
            tangent = DirectX::XMVector3Normalize(tangent);
            DirectX::XMStoreFloat3(&vertex.Tangent, tangent);

            // 纹理坐标
            vertex.Texc.x = (float)j / sliceCount;
            vertex.Texc.y = (float)i / stackCount;

            meshData.vertices.push_back(vertex);
        }
    }

    // 底部顶点
    Vertex bottomVertex;
    bottomVertex.Position = DirectX::XMFLOAT3(0.0f, -radius, 0.0f);
    bottomVertex.Normal = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);
    bottomVertex.Tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
    bottomVertex.Texc = DirectX::XMFLOAT2(0.0f, 1.0f);
    meshData.vertices.push_back(bottomVertex);

    // 生成顶部索引
    for (UINT i = 0; i < sliceCount; ++i)
    {
        meshData.indices.push_back(0);
        meshData.indices.push_back(1 + i);
        meshData.indices.push_back(1 + (i + 1) % sliceCount);
    }

    // 生成中间层索引
    UINT baseIndex = 1;
    UINT ringVertexCount = sliceCount + 1;
    for (UINT i = 0; i < stackCount - 2; ++i)
    {
        for (UINT j = 0; j < sliceCount; ++j)
        {
            meshData.indices.push_back(baseIndex + i * ringVertexCount + j);
            meshData.indices.push_back(baseIndex + i * ringVertexCount + j + 1);
            meshData.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

            meshData.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
            meshData.indices.push_back(baseIndex + i * ringVertexCount + j + 1);
            meshData.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
        }
    }

    // 生成底部索引
    UINT southPoleIndex = static_cast<UINT>(meshData.vertices.size() - 1);
    baseIndex = southPoleIndex - ringVertexCount;
    for (UINT i = 0; i < sliceCount; ++i)
    {
        meshData.indices.push_back(southPoleIndex);
        meshData.indices.push_back(baseIndex + i);
        meshData.indices.push_back(baseIndex + (i + 1) % sliceCount);
    }

    return meshData;
}