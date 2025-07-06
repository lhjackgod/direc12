cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProj;
};

struct ObjectConsts
{
    float4x4 gWorldViewProj;
    uint matIndex;
};
ConstantBuffer<ObjectConsts> gObjectConstants : register(b0);
uint index = gObjectConstants.matIndex;
struct VertexIn
{
    float3 PosL : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float4 Color : COLOR:
};

void VS(VertexIn vin)
{
    VertexOut vout;
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
    vout.Color = vin.Color;
    return vout;       
}

float4 PS(VertexOut pin) : SV_Target
{
    return vout.Color;
}
// or (float4 PosH : SV_POSITION, float4 color : COLOR) : SV_Target