#include "pch.h"
#include <UniDx/Material.h>

#include <UniDx/D3DManager.h>
#include <UniDx/Texture.h>
#include <UniDx/ConstantBuffer.h>


namespace UniDx{

// BlendMode に対応する D3D11_RENDER_TARGET_BLEND_DESC の定数配列
namespace
{
    static const D3D11_RENDER_TARGET_BLEND_DESC blendModeRTDesc[] = {
        // BlendMode_Opaque
        { FALSE,
            D3D11_BLEND_ONE,           // SrcBlend
            D3D11_BLEND_ZERO,          // DestBlend
            D3D11_BLEND_OP_ADD,        // BlendOp
            D3D11_BLEND_ONE,           // SrcBlendAlpha
            D3D11_BLEND_ZERO,          // DestBlendAlpha
            D3D11_BLEND_OP_ADD,        // BlendOpAlpha
            D3D11_COLOR_WRITE_ENABLE_ALL },

        // BlendMode_Alpha (通常のアルファブレンド: SrcAlpha, 1-SrcAlpha)
        { TRUE,
            D3D11_BLEND_SRC_ALPHA,
            D3D11_BLEND_INV_SRC_ALPHA,
            D3D11_BLEND_OP_ADD,
            D3D11_BLEND_ONE,
            D3D11_BLEND_ZERO,
            D3D11_BLEND_OP_ADD,
            D3D11_COLOR_WRITE_ENABLE_ALL },

        // BlendMode_PremultipliedAlpha (前乗算アルファ: One, 1-SrcAlpha)
        { TRUE,
            D3D11_BLEND_ONE,
            D3D11_BLEND_INV_SRC_ALPHA,
            D3D11_BLEND_OP_ADD,
            D3D11_BLEND_ONE,
            D3D11_BLEND_INV_SRC_ALPHA,
            D3D11_BLEND_OP_ADD,
            D3D11_COLOR_WRITE_ENABLE_ALL },

        // BlendMode_Additive (加算: SrcAlpha, One)
        { TRUE,
            D3D11_BLEND_SRC_ALPHA,
            D3D11_BLEND_ONE,
            D3D11_BLEND_OP_ADD,
            D3D11_BLEND_ONE,
            D3D11_BLEND_ONE,
            D3D11_BLEND_OP_ADD,
            D3D11_COLOR_WRITE_ENABLE_ALL },
    };
    static_assert(std::size(blendModeRTDesc) == 4, "Blend mode mapping size must match BlendMode enum count");
} // namespace


// -----------------------------------------------------------------------------
// コンストラクタ
// -----------------------------------------------------------------------------
Material::Material() :
    Object([this]() { return shader.name; }),
    color(1, 1, 1, 1),
    mainTexture(
        [this]() { return textures.size() > 0 ? textures.front().get() : nullptr; }
    ),
    depthWrite(D3D11_DEPTH_WRITE_MASK_ALL), // デフォルトは書き込み有効
    ztest(D3D11_COMPARISON_LESS), // デフォルトは小さい値が手前
    renderingMode(RenderingMode_Opaque), // デフォルトは不透明
    blendMode(BlendMode_Alpha) // デフォルトは標準ブレンド
{
}


// -----------------------------------------------------------------------------
// レンダリング用にデバイスへ設定
// -----------------------------------------------------------------------------
bool Material::setForRender() const
{
    // レンダリングモードが合わない場合は何もしない
    // レンダーキューを実装していない代わりの実装
    if (renderingMode != D3DManager::getInstance()->getCurrentRenderingMode())
    {
        return false;
    }

    shader.setToContext();
    for (auto& tex : textures)
    {
        tex->setForRender();
    }

    // デプス
    D3DManager::getInstance()->GetContext()->OMSetDepthStencilState(depthStencilState.Get(), 1);

    // ブレンド
    D3DManager::getInstance()->GetContext()->OMSetBlendState(blendState.Get(), NULL, 0xffffffff);

    // 定数バッファ更新
    ConstantBufferPerMaterial cb{};
    cb.baseColor = color;

    ID3D11Buffer* cbs[1] = { constantBufferPerMaterial.Get() };
    D3DManager::getInstance()->GetContext()->VSSetConstantBuffers(CB_PerMaterial, 1, cbs);
    D3DManager::getInstance()->GetContext()->PSSetConstantBuffers(CB_PerMaterial, 1, cbs);
    D3DManager::getInstance()->GetContext()->UpdateSubresource(constantBufferPerMaterial.Get(), 0, nullptr, &cb, 0, 0);

    return true;
}


// -----------------------------------------------------------------------------
// テクスチャ追加
// -----------------------------------------------------------------------------
void Material::AddTexture(std::shared_ptr<Texture> tex)
{
    textures.push_back(tex);
}


// -----------------------------------------------------------------------------
// 有効化
// -----------------------------------------------------------------------------
void Material::OnEnable()
{
    // デプスステート作成
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE; // 深度テスト有効
    dsDesc.DepthWriteMask = depthWrite; // 書き込み有効
    dsDesc.DepthFunc = ztest; // 小さい値が手前

    dsDesc.StencilEnable = FALSE;

    D3DManager::getInstance()->GetDevice()->CreateDepthStencilState(&dsDesc, &depthStencilState);

    // ブレンドステート作成
    setBlendMode(blendMode);

    // マテリアル用の定数バッファ生成
    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth = sizeof(ConstantBufferPerMaterial);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    D3DManager::getInstance()->GetDevice()->CreateBuffer(&desc, nullptr, constantBufferPerMaterial.GetAddressOf());
}


// -----------------------------------------------------------------------------
// ブレンドモードを指定してブレンドステートを作成
// -----------------------------------------------------------------------------
void Material::setBlendMode(BlendMode e)
{
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0] = blendModeRTDesc[e];
    D3DManager::getInstance()->GetDevice()->CreateBlendState(&blendDesc, &blendState);
}


}
