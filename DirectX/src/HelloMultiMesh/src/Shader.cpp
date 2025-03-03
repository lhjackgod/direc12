#include "pch.h"
#include "Shader.h"

void Shader::OnInit(const LPCWSTR& vShaderPath, const LPCWSTR& pShaderPath,
        const D3D12_INPUT_ELEMENT_DESC* inputElement, UINT inputElementCount,
        const CD3DX12_ROOT_PARAMETER1* rootParameter, UINT numParamters,
        UINT numStaticSample, const D3D12_STATIC_SAMPLER_DESC* staticSamplerDesc,
        const ComPtr<ID3D12Device>& device)
{
    ComPtr<ID3DBlob> vError;
    D3DCompileFromFile(vShaderPath, nullptr, nullptr, "VSMain", "vs_5_0", D3DCOMPILE_DEBUG, 0, vShader.GetAddressOf(),
        vError.GetAddressOf());
    if (vError!=nullptr)
    {
        const char* vErrorMessage = static_cast<const char*>(vError->GetBufferPointer());
        throw std::runtime_error(vErrorMessage);
    }
    ComPtr<ID3DBlob> pError;
    D3DCompileFromFile(pShaderPath, nullptr, nullptr, "PSMain", "ps_5_0", D3DCOMPILE_DEBUG, 0,
        pShader.GetAddressOf(),pError.GetAddressOf());
    if (pError != nullptr)
    {
        const char* pErrorMessage = static_cast<const char*>(pError->GetBufferPointer());
        throw std::runtime_error(pErrorMessage);
    }

    //root signature
    {
        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionRootSignatureDesc{};
        versionRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &versionRootSignatureDesc, sizeof(versionRootSignatureDesc))))
        {
            versionRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }
        versionRootSignatureDesc.Init_1_1(numParamters, rootParameter, numStaticSample,
            staticSamplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeVersionedRootSignature(&versionRootSignatureDesc, signature.GetAddressOf(), error.GetAddressOf()));
        ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.GetAddressOf())));
    }

    //pipline state
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
        desc.pRootSignature = m_RootSignature.Get();
        desc.VS = CD3DX12_SHADER_BYTECODE(vShader.Get());
        desc.PS = CD3DX12_SHADER_BYTECODE(pShader.Get());
        desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        desc.SampleMask = UINT_MAX;
        desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        desc.InputLayout = {inputElement, inputElementCount};
        desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets = 1;
        desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        ThrowIfFailed(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(piplineState.GetAddressOf())));
    }
}
