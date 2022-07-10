#include "LightHelper.hlsli"

Texture2D g_Tex : register(t0); //Texture2D
SamplerState g_SamLinear : register(s0);
//9-2
//Texture2DArray j_TexArray : register(t1);
Texture2D j_Tex1 : register(t1);
Texture2D j_Tex2 : register(t2);
Texture2D j_Tex3 : register(t3);
Texture2D j_Tex4 : register(t4);
Texture2D j_Tex5 : register(t5);
Texture2D j_Tex6 : register(t6);

cbuffer VSConstantBuffer : register(b0)
{
    matrix g_World; 
    matrix g_View;  
    matrix g_Proj;  
    matrix g_WorldInvTranspose;
}

cbuffer PSConstantBuffer : register(b1)
{
    DirectionalLight g_DirLight[10];
    PointLight g_PointLight[10];
    SpotLight g_SpotLight[10];
    Material g_Material;
    int g_NumDirLight;
    int g_NumPointLight;
    int g_NumSpotLight;
    float g_Pad1;

    float3 g_EyePosW;
    float g_Pad2;
}

//VS的 in
struct VertexPosNormalTex
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
    //9-2
    //uint face_id : FACEID;
};

struct VertexPosTex
{
    float3 PosL : POSITION;
    float2 Tex : TEXCOORD;
};

//VS 的 out, PS的 IN
struct VertexPosHWNormalTex
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;     // 在世界中的位置
    float3 NormalW : NORMAL;    // 法向量在世界中的方向
    float2 Tex : TEXCOORD;
    //9-2
    //uint PrimID : SV_PrimitiveID;
};

struct VertexPosHTex
{
    float4 PosH : SV_POSITION;
    float2 Tex : TEXCOORD;
};










