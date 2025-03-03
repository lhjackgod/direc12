#pragma once

class Shader
{
public:
    Shader() = default;
    void OnInit(const LPCWSTR& vShaderPath, const LPCWSTR& pShaderPath,
        const D3D12_INPUT_ELEMENT_DESC* inputElement, UINT inputElementCount,
        const CD3DX12_ROOT_PARAMETER1* rootParameter, UINT numParamters,
        UINT numStaticSample, const D3D12_STATIC_SAMPLER_DESC* staticSamplerDesc,
        const ComPtr<ID3D12Device>& device);
    ComPtr<ID3D12PipelineState> getPipelineSate() { return piplineState; }
    ComPtr<ID3D12RootSignature> getRootSignature() { return m_RootSignature; }
private:
    ComPtr<ID3DBlob> vShader;
    ComPtr<ID3DBlob> pShader;
    ComPtr<ID3D12PipelineState> piplineState;
    ComPtr<ID3D12RootSignature> m_RootSignature;
};
