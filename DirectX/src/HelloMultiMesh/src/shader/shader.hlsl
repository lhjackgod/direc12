struct vShaderInput
{
    float3 aPos : POSITION;
    float4 aColor : COLOR;
};

struct pShaderInput
{
    float4 aPos : SV_POSITION;
    float4 aColor : COLOR;
};

pShaderInput VSMain(vShaderInput input)
{
    pShaderInput output;
    output.aPos = float4(input.aPos, 1.0f);
    output.aColor = input.aColor;
    return output;
}

float4 PSMain(pShaderInput input) : SV_TARGET
{
    return input.aColor;
}