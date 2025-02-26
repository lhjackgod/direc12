cbuffer ConstantBuffer : register(b0)
{
    float4x4 proj;
};

    struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};
struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};
PSInput VSMain(VSInput input)
{
    PSInput output;

    output.position = mul(float4(input.position, 1.0), proj);
    output.color = input.color;

    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 color = input.color;
    return color;
}