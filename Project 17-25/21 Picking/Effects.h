//***************************************************************************************
// Effects.h by X_Jun(MKXJun) (C) 2018-2022 All Rights Reserved.
// Licensed under the MIT License.
//
// 简易特效管理框架
// Simple effect management framework.
//***************************************************************************************

#ifndef EFFECTS_H
#define EFFECTS_H
#include <memory>
#include "LightHelper.h"
#include "RenderStates.h"


class IEffect
{
public:
    // 使用模板别名(C++11)简化类型名
    template <class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    IEffect() = default;
    virtual ~IEffect() = default;
    // 不允许拷贝，允许移动
    IEffect(const IEffect&) = delete;
    IEffect& operator=(const IEffect&) = delete;
    IEffect(IEffect&&) = default;
    IEffect& operator=(IEffect&&) = default;

    // 更新并绑定常量缓冲区
    virtual void Apply(ID3D11DeviceContext * deviceContext) = 0;
};


class BasicEffect : public IEffect
{
public:

    enum RenderType { RenderObject, RenderInstance };

    BasicEffect();
    virtual ~BasicEffect() override;

    BasicEffect(BasicEffect&& moveFrom) noexcept;
    BasicEffect& operator=(BasicEffect&& moveFrom) noexcept;

    // 获取单例
    static BasicEffect& Get();

    

    // 初始化Basic.fx所需资源并初始化渲染状态
    bool InitAll(ID3D11Device * device);


    // 
    // 渲染模式的变更
    //

    // 默认状态来绘制
    void SetRenderDefault(ID3D11DeviceContext * deviceContext, RenderType type);
    
    //
    // 矩阵设置
    //

    void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W);
    void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V);
    void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P);
    
    //
    // 光照、材质和纹理相关设置
    //

    // 各种类型灯光允许的最大数目
    static const int maxLights = 5;

    void SetDirLight(size_t pos, const DirectionalLight& dirLight);
    void SetPointLight(size_t pos, const PointLight& pointLight);
    void SetSpotLight(size_t pos, const SpotLight& spotLight);

    void SetMaterial(const Material& material);

    void SetTextureDiffuse(ID3D11ShaderResourceView * textureDiffuse);
    

    void SetEyePos(const DirectX::XMFLOAT3& eyePos);
    

    // 应用常量缓冲区和纹理资源的变更
    void Apply(ID3D11DeviceContext * deviceContext) override;
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};










#endif
