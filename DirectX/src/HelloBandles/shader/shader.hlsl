struct VSInput{
    float4 aPos : SV_POSITION;
    float4 aColor : COLOR;
};

VSInput VSmain(float3 aPos : POSITION, float4 aColor : COLOR)
{
    VSInput input;
    input.aPos = float4(aPos, 1.0);
    input.aColor = aColor;
    return input;
}

float4 PSmain(VSInput input) : SV_TARGET
{
    return input.aColor;
}