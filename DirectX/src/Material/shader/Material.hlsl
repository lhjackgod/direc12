struct VSInput
{
    float4 position : SV_POSITION;
    float2 UV : TEXCOORD;
};

Texture2D gTexture : register(t0);
SamplerState g_Sampler : register(s0);
VSInput VSMain(float3 position : POSITION, float4 uv : TEXCOORD)
{
    VSInput input;
    input.position = float4(position, 1.0);
    input.UV = uv;

    return input;
}

float4 PSMain(VSInput input) : SV_TARGET
{
    return gTexture.Sample(g_Sampler, input.UV);
}