cbuffer ConstantBuffer : register(b0)
{
    float4 time;
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
    input.position.xy += 0.5f * sin(input.position.x) * sin(3.0f * time.x);
    input.position.z *= 0.6f + 0.4f * sin(2.0f * time.x);
    output.position = mul(float4(input.position, 1.0), proj);
    output.color = input.color;

    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 color = input.color;
    return color;
}