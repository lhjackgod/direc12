struct VSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

cbuffer FirstConstantBuffer
{
    float4 offset;
    float4 padding[15];
};

Texture2D gTexture : register(t0);
SamplerState g_Sampler : register(s0);
VSInput VSMain(float4 position : POSITION, float4 uv : TEXCOORD)
{
    VSInput input;
    input.position = position + offset;
    input.uv = uv;

    return input;
}

float4 PSMain(VSInput input) : SV_TARGET
{
    return gTexture.Sample(g_Sampler, input.uv);
}