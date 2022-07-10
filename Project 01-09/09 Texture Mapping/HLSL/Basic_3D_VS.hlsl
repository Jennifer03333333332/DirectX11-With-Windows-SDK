#include "Basic.hlsli"

// 顶点着色器(3D)
VertexPosHWNormalTex VS(VertexPosNormalTex vIn)
{
    VertexPosHWNormalTex vOut;
    matrix viewProj = mul(g_View, g_Proj);
    float4 posW = mul(float4(vIn.PosL, 1.0f), g_World);

    vOut.PosH = mul(posW, viewProj);
    vOut.PosW = posW.xyz;
    vOut.NormalW = mul(vIn.NormalL, (float3x3) g_WorldInvTranspose);
    vOut.Tex = mul(float4(vIn.Tex.x - 0.5f, vIn.Tex.y - 0.5f, 0.0f, 1.0f), g_RotationMat).xy; //vIn.Tex;
    vOut.Tex = float2(vOut.Tex.x + 0.5f, vOut.Tex.y + 0.5f);
    return vOut;
}
