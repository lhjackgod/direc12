struct VSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VSInput VSMain(float3 position : POSITION, float4 color : COLOR)
{
    VSInput input;
    input.position = float4(position, 1.0);
    input.color = color;

    return input;
}

float4 PSMain(VSInput input) : SV_TARGET
{
    return input.color;
}