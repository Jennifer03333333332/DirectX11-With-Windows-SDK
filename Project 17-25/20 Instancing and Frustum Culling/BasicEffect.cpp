#include "Effects.h"
#include "d3dUtil.h"
#include "EffectHelper.h"	// 必须晚于Effects.h和d3dUtil.h包含
#include "DXTrace.h"
#include "Vertex.h"
using namespace DirectX;

#pragma warning(disable: 26812)

//
// BasicEffect::Impl 需要先于BasicEffect的定义
//

class BasicEffect::Impl : public AlignedType<BasicEffect::Impl>
{
public:

    //
    // 这些结构体对应HLSL的结构体。需要按16字节对齐
    //

    struct CBChangesEveryInstanceDrawing
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX worldInvTranspose;
    };

    struct CBChangesEveryObjectDrawing
    {
        Material material;
    };

    struct CBChangesEveryFrame
    {
        DirectX::XMMATRIX viewProj;
        DirectX::XMFLOAT3 eyePos;
        float pad;
    };

    struct CBChangesRarely
    {
        DirectionalLight dirLight[BasicEffect::maxLights];
        PointLight pointLight[BasicEffect::maxLights];
        SpotLight spotLight[BasicEffect::maxLights];
    };


public:
    // 必须显式指定
    Impl() : m_IsDirty() {}
    ~Impl() = default;

public:
    // 需要16字节对齐的优先放在前面
    XMMATRIX m_View{}, m_Proj{};
    CBufferObject<0, CBChangesEveryInstanceDrawing>	m_CBInstDrawing;		// 每次实例绘制的常量缓冲区
    CBufferObject<1, CBChangesEveryObjectDrawing>	m_CBObjDrawing;		    // 每次对象绘制的常量缓冲区
    CBufferObject<2, CBChangesEveryFrame>			m_CBFrame;			    // 每帧绘制的常量缓冲区
    CBufferObject<3, CBChangesRarely>				m_CBRarely;			    // 几乎不会变更的常量缓冲区
    BOOL m_IsDirty;											                // 是否有值变更
    std::vector<CBufferBase*> m_pCBuffers;					                // 统一管理上面所有的常量缓冲区


    ComPtr<ID3D11VertexShader> m_pBasicInstanceVS;
    ComPtr<ID3D11VertexShader> m_pBasicObjectVS;

    ComPtr<ID3D11PixelShader> m_pBasicPS;

    ComPtr<ID3D11InputLayout> m_pInstancePosNormalTexLayout;	
    ComPtr<ID3D11InputLayout> m_pVertexPosNormalTexLayout;		

    ComPtr<ID3D11ShaderResourceView> m_pTextureDiffuse;		                // 漫反射紋理
};

//
// BasicEffect
//

namespace
{
    // BasicEffect单例
    static BasicEffect * g_pInstance = nullptr;
}

BasicEffect::BasicEffect()
{
    if (g_pInstance)
        throw std::exception("BasicEffect is a singleton!");
    g_pInstance = this;
    pImpl = std::make_unique<BasicEffect::Impl>();
}

BasicEffect::~BasicEffect()
{
}

BasicEffect::BasicEffect(BasicEffect && moveFrom) noexcept
{
    pImpl.swap(moveFrom.pImpl);
}

BasicEffect & BasicEffect::operator=(BasicEffect && moveFrom) noexcept
{
    pImpl.swap(moveFrom.pImpl);
    return *this;
}

BasicEffect & BasicEffect::Get()
{
    if (!g_pInstance)
        throw std::exception("BasicEffect needs an instance!");
    return *g_pInstance;
}


bool BasicEffect::InitAll(ID3D11Device * device)
{
    if (!device)
        return false;

    if (!pImpl->m_pCBuffers.empty())
        return true;

    if (!RenderStates::IsInit())
        throw std::exception("RenderStates need to be initialized first!");

    ComPtr<ID3DBlob> blob;

    // 实例输入布局
    D3D11_INPUT_ELEMENT_DESC basicInstLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "World", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        { "World", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        { "World", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        { "World", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        { "WorldInvTranspose", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        { "WorldInvTranspose", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        { "WorldInvTranspose", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        { "WorldInvTranspose", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 112, D3D11_INPUT_PER_INSTANCE_DATA, 1}
    };

    //
    // 创建顶点着色器
    //

    HR(CreateShaderFromFile(L"HLSL\\BasicInstance_VS.cso", L"HLSL\\BasicInstance_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pBasicInstanceVS.GetAddressOf()));
    // 创建顶点布局
    HR(device->CreateInputLayout(basicInstLayout, ARRAYSIZE(basicInstLayout),
        blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pInstancePosNormalTexLayout.GetAddressOf()));

    HR(CreateShaderFromFile(L"HLSL\\BasicObject_VS.cso", L"HLSL\\BasicObject_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pBasicObjectVS.GetAddressOf()));
    // 创建顶点布局
    HR(device->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
        blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexPosNormalTexLayout.GetAddressOf()));

    //
    // 创建像素着色器
    //

    HR(CreateShaderFromFile(L"HLSL\\Basic_PS.cso", L"HLSL\\Basic_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
    HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pBasicPS.GetAddressOf()));

    pImpl->m_pCBuffers.assign({
        &pImpl->m_CBInstDrawing,
        &pImpl->m_CBObjDrawing, 
        &pImpl->m_CBFrame, 
        &pImpl->m_CBRarely});

    // 创建常量缓冲区
    for (auto& pBuffer : pImpl->m_pCBuffers)
    {
        HR(pBuffer->CreateBuffer(device));
    }

    // 设置调试对象名
    D3D11SetDebugObjectName(pImpl->m_pInstancePosNormalTexLayout.Get(), "InstancePosNormalTexLayout");
    D3D11SetDebugObjectName(pImpl->m_pVertexPosNormalTexLayout.Get(), "VertexPosNormalTexLayout");
    D3D11SetDebugObjectName(pImpl->m_pCBuffers[0]->cBuffer.Get(), "CBInstDrawing");
    D3D11SetDebugObjectName(pImpl->m_pCBuffers[1]->cBuffer.Get(), "CBObjDrawing");
    D3D11SetDebugObjectName(pImpl->m_pCBuffers[2]->cBuffer.Get(), "CBFrame");
    D3D11SetDebugObjectName(pImpl->m_pCBuffers[3]->cBuffer.Get(), "CBRarely");
    D3D11SetDebugObjectName(pImpl->m_pBasicObjectVS.Get(), "BasicObject_VS");
    D3D11SetDebugObjectName(pImpl->m_pBasicInstanceVS.Get(), "BasicInstance_VS");
    D3D11SetDebugObjectName(pImpl->m_pBasicPS.Get(), "Basic_PS");

    return true;
}


void BasicEffect::SetRenderDefault(ID3D11DeviceContext * deviceContext, RenderType type)
{
    if (type == RenderInstance)
    {
        deviceContext->IASetInputLayout(pImpl->m_pInstancePosNormalTexLayout.Get());
        deviceContext->VSSetShader(pImpl->m_pBasicInstanceVS.Get(), nullptr, 0);
        deviceContext->PSSetShader(pImpl->m_pBasicPS.Get(), nullptr, 0);
    }
    else
    {
        deviceContext->IASetInputLayout(pImpl->m_pVertexPosNormalTexLayout.Get());
        deviceContext->VSSetShader(pImpl->m_pBasicObjectVS.Get(), nullptr, 0);
        deviceContext->PSSetShader(pImpl->m_pBasicPS.Get(), nullptr, 0);
    }

    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    deviceContext->GSSetShader(nullptr, nullptr, 0);
    deviceContext->RSSetState(nullptr);
    
    deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
    deviceContext->OMSetDepthStencilState(nullptr, 0);
    deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void XM_CALLCONV BasicEffect::SetWorldMatrix(DirectX::FXMMATRIX W)
{
    auto& cBuffer = pImpl->m_CBInstDrawing;
    cBuffer.data.world = XMMatrixTranspose(W);
    cBuffer.data.worldInvTranspose = XMMatrixTranspose(InverseTranspose(W));
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicEffect::SetViewMatrix(FXMMATRIX V)
{
    auto& cBuffer = pImpl->m_CBFrame;
    pImpl->m_View = V;
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicEffect::SetProjMatrix(FXMMATRIX P)
{
    auto& cBuffer = pImpl->m_CBFrame;
    pImpl->m_Proj = P;
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetDirLight(size_t pos, const DirectionalLight & dirLight)
{
    auto& cBuffer = pImpl->m_CBRarely;
    cBuffer.data.dirLight[pos] = dirLight;
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetPointLight(size_t pos, const PointLight & pointLight)
{
    auto& cBuffer = pImpl->m_CBRarely;
    cBuffer.data.pointLight[pos] = pointLight;
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetSpotLight(size_t pos, const SpotLight & spotLight)
{
    auto& cBuffer = pImpl->m_CBRarely;
    cBuffer.data.spotLight[pos] = spotLight;
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetMaterial(const Material & material)
{
    auto& cBuffer = pImpl->m_CBObjDrawing;
    cBuffer.data.material = material;
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetTextureDiffuse(ID3D11ShaderResourceView * textureDiffuse)
{
    pImpl->m_pTextureDiffuse = textureDiffuse;
}

void BasicEffect::SetEyePos(const DirectX::XMFLOAT3& eyePos)
{
    auto& cBuffer = pImpl->m_CBFrame;
    cBuffer.data.eyePos = eyePos;
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::Apply(ID3D11DeviceContext * deviceContext)
{
    pImpl->m_CBFrame.data.viewProj = XMMatrixTranspose(pImpl->m_View * pImpl->m_Proj);

    auto& pCBuffers = pImpl->m_pCBuffers;
    // 将缓冲区绑定到渲染管线上
    pCBuffers[0]->BindVS(deviceContext);
    pCBuffers[2]->BindVS(deviceContext);

    pCBuffers[1]->BindPS(deviceContext);
    pCBuffers[2]->BindPS(deviceContext);
    pCBuffers[3]->BindPS(deviceContext);

    // 设置纹理
    deviceContext->PSSetShaderResources(0, 1, pImpl->m_pTextureDiffuse.GetAddressOf());

    if (pImpl->m_IsDirty)
    {
        pImpl->m_IsDirty = false;
        for (auto& pCBuffer : pCBuffers)
        {
            pCBuffer->UpdateBuffer(deviceContext);
        }
    }
}

