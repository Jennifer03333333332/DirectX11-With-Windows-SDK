#include "Basic.hlsli"

// 像素着色器
float4 PS(VertexPosHWNormalTex pIn) : SV_Target
{
    // 提前进行Alpha裁剪，对不符合要求的像素可以避免后续运算
    float4 texColor = g_Tex.Sample(g_Sam, pIn.Tex);
    clip(texColor.a - 0.05f);

    // 标准化法向量
    pIn.NormalW = normalize(pIn.NormalW);

    // 求出顶点指向眼睛的向量，以及顶点与眼睛的距离
    float3 toEyeW = normalize(g_EyePosW - pIn.PosW);
    float distToEye = distance(g_EyePosW, pIn.PosW);

    // 初始化为0 
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);

    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        ComputeDirectionalLight(g_Material, g_DirLight[i], pIn.NormalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }
    
    float4 litColor = texColor * (ambient + diffuse) + spec;

    // 雾效部分
    [flatten]
    if (g_FogEnabled)
    {
        // 限定在0.0f到1.0f范围
        float fogLerp = saturate((distToEye - g_FogStart) / g_FogRange);
        // 根据雾色和光照颜色进行线性插值
        litColor = lerp(litColor, g_FogColor, fogLerp);
    }

    litColor.a = texColor.a * g_Material.Diffuse.a;
    return litColor;
}
