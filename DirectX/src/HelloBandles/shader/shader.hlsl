
cbuffer objectViewMatrix : register(b0)
{
    float4x4 proj;
    float4 move;
    float4 padding[8];
};

struct VSInput{
    float4 aPos : SV_POSITION;
    float4 aColor : COLOR;
};

VSInput VSmain(float3 aPos : POSITION, float4 aColor : COLOR)
{
    VSInput input;
    input.aPos = mul(float4(aPos, 1.0f)
    ,proj);
    input.aColor = aColor;
    return input;
}

float4 PSmain(VSInput input) : SV_TARGET
{
    return input.aColor;
}
