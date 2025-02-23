#include "Effects.h"
#include "d3dUtil.h"
#include "EffectHelper.h"	// 必须晚于Effects.h和d3dUtil.h包含
#include "DXTrace.h"
#include "Vertex.h"
using namespace DirectX;


//
// ScreenFadeEffect::Impl 需要先于ScreenFadeEffect的定义
//

class ScreenFadeEffect::Impl : public AlignedType<ScreenFadeEffect::Impl>
{
public:

    //
    // 这些结构体对应HLSL的结构体。需要按16字节对齐
    //

    struct CBChangesEveryFrame
    {
        float fadeAmount;
        XMFLOAT3 pad;
    };

    struct CBChangesRarely
    {
        XMMATRIX worldViewProj;
    };


public:
    // 必须显式指定
    Impl() : m_IsDirty() {}
    ~Impl() = default;

public:
    CBufferObject<0, CBChangesEveryFrame> m_CBFrame;		// 每帧修改的常量缓冲区
    CBufferObject<1, CBChangesRarely>	m_CBRarely;		    // 很少修改的常量缓冲区
    

    BOOL m_IsDirty;										    // 是否有值变更
    std::vector<CBufferBase*> m_pCBuffers;				    // 统一管理上面所有的常量缓冲区

    ComPtr<ID3D11VertexShader> m_pScreenFadeVS;
    ComPtr<ID3D11PixelShader> m_pScreenFadePS;

    ComPtr<ID3D11InputLayout> m_pVertexPosTexLayout;

    ComPtr<ID3D11ShaderResourceView> m_pTexture;			// 用于淡入淡出的纹理
};



//
// ScreenFadeEffect
//

namespace
{
    // ScreenFadeEffect单例
    static ScreenFadeEffect * g_pInstance = nullptr;
}

ScreenFadeEffect::ScreenFadeEffect()
{
    if (g_pInstance)
        throw std::exception("ScreenFadeEffect is a singleton!");
    g_pInstance = this;
    pImpl = std::make_unique<ScreenFadeEffect::Impl>();
}

ScreenFadeEffect::~ScreenFadeEffect()
{
}

ScreenFadeEffect::ScreenFadeEffect(ScreenFadeEffect && moveFrom) noexcept
{
    pImpl.swap(moveFrom.pImpl);
}

ScreenFadeEffect & ScreenFadeEffect::operator=(ScreenFadeEffect && moveFrom) noexcept
{
    pImpl.swap(moveFrom.pImpl);
    return *this;
}

ScreenFadeEffect & ScreenFadeEffect::Get()
{
    if (!g_pInstance)
        throw std::exception("ScreenFadeEffect needs an instance!");
    return *g_pInstance;
}

bool ScreenFadeEffect::InitAll(ID3D11Device * device)
{
    if (!device)
        return false;

    if (!pImpl->m_pCBuffers.empty())
        return true;

    if (!RenderStates::IsInit())
        throw std::exception("RenderStates need to be initialized first!");

    ComPtr<ID3DBlob> blob;

    // ******************
    // 创建顶点着色器
    //

    HR(CreateShaderFromFile(L"HLSL\\ScreenFade_VS.cso", L"HLSL\\ScreenFade_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pScreenFadeVS.GetAddressOf()));
    // 创建顶点布局
    HR(device->CreateInputLayout(VertexPosTex::inputLayout, ARRAYSIZE(VertexPosTex::inputLayout),
        blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexPosTexLayout.GetAddressOf()));

    // ******************
    // 创建像素着色器
    //

    HR(CreateShaderFromFile(L"HLSL\\ScreenFade_PS.cso", L"HLSL\\ScreenFade_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
    HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pScreenFadePS.GetAddressOf()));



    pImpl->m_pCBuffers.assign({
        &pImpl->m_CBFrame,
        &pImpl->m_CBRarely
        });

    // 创建常量缓冲区
    for (auto& pBuffer : pImpl->m_pCBuffers)
    {
        HR(pBuffer->CreateBuffer(device));
    }

    // 设置调试对象名
    D3D11SetDebugObjectName(pImpl->m_pVertexPosTexLayout.Get(), "ScreenFadeEffect.VertexPosTexLayout");
    D3D11SetDebugObjectName(pImpl->m_pCBuffers[0]->cBuffer.Get(), "ScreenFadeEffect.CBFrame");
    D3D11SetDebugObjectName(pImpl->m_pCBuffers[1]->cBuffer.Get(), "ScreenFadeEffect.CBRarely");
    D3D11SetDebugObjectName(pImpl->m_pScreenFadeVS.Get(), "ScreenFadeEffect.ScreenFade_VS");
    D3D11SetDebugObjectName(pImpl->m_pScreenFadePS.Get(), "ScreenFadeEffect.ScreenFade_PS");

    return true;
}

void ScreenFadeEffect::SetRenderDefault(ID3D11DeviceContext * deviceContext)
{
    deviceContext->IASetInputLayout(pImpl->m_pVertexPosTexLayout.Get());
    deviceContext->VSSetShader(pImpl->m_pScreenFadeVS.Get(), nullptr, 0);
    deviceContext->PSSetShader(pImpl->m_pScreenFadePS.Get(), nullptr, 0);

    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    deviceContext->GSSetShader(nullptr, nullptr, 0);
    deviceContext->RSSetState(nullptr);

    deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
    deviceContext->OMSetDepthStencilState(nullptr, 0);
    deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void XM_CALLCONV ScreenFadeEffect::SetWorldViewProjMatrix(DirectX::FXMMATRIX W, DirectX::CXMMATRIX V, DirectX::CXMMATRIX P)
{
    auto& cBuffer = pImpl->m_CBRarely;
    cBuffer.data.worldViewProj = XMMatrixTranspose(W * V * P);
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV ScreenFadeEffect::SetWorldViewProjMatrix(DirectX::FXMMATRIX WVP)
{
    auto& cBuffer = pImpl->m_CBRarely;
    cBuffer.data.worldViewProj = XMMatrixTranspose(WVP);
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void ScreenFadeEffect::SetFadeAmount(float fadeAmount)
{
    auto& cBuffer = pImpl->m_CBFrame;
    cBuffer.data.fadeAmount = fadeAmount;
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}


void ScreenFadeEffect::SetTexture(ID3D11ShaderResourceView * texture)
{
    pImpl->m_pTexture = texture;
}

void ScreenFadeEffect::Apply(ID3D11DeviceContext * deviceContext)
{
    auto& pCBuffers = pImpl->m_pCBuffers;
    // 将缓冲区绑定到渲染管线上
    pCBuffers[0]->BindPS(deviceContext);
    pCBuffers[1]->BindVS(deviceContext);
    // 设置SRV
    deviceContext->PSSetShaderResources(0, 1, pImpl->m_pTexture.GetAddressOf());

    if (pImpl->m_IsDirty)
    {
        pImpl->m_IsDirty = false;
        for (auto& pCBuffer : pCBuffers)
        {
            pCBuffer->UpdateBuffer(deviceContext);
        }
    }
}
