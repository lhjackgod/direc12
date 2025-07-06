struct vShaderInput
{
    float3 aPos : POSITION;
	float3 aNormal : NORMAL;
	float3 aTangent : TANGENT;
    float2 aUV : TEXCOORD;
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
	output.aColor = float4(1,1,1,1);
    return output;
}

float4 PSMain(pShaderInput input) : SV_TARGET
{
    return float4(1,1,1,1);
}