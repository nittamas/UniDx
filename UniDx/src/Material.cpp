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
    Object([this]() { return shader->name; }),
    shader(make_shared<Shader>()),
    color(1, 1, 1, 1),
    mainTexture(
        [this]() { return textures.size() > 0 ? textures.front().get() : nullptr; }
    ),
    depthWrite(D3D11_DEPTH_WRITE_MASK_ALL), // デフォルトは書き込み有効
    ztest(D3D11_COMPARISON_LESS), // デフォルトは小さい値が手前
    cullMode(D3D11_CULL_BACK), // デフォルトは裏面非表示
    renderingMode(RenderingMode_Opaque), // デフォルトは不透明
    blendMode(BlendMode_Alpha) // デフォルトは標準ブレンド
{
}


// マテリアル変数を取得
const void* Material::GetBytes(StringId name) const
{
    if(shader == nullptr) return nullptr;

    // シェーダーから該当する名前のレイアウトを取得
    const auto* layout = shader->findVar(name);
    if(!layout) return nullptr;

    // シェーダーから該当する名前のレイアウトを取得
    const uint32_t cbSize = shader->getCBPerMaterialSize();
    if(cbStaging.size() != cbSize) nullptr;

    return cbStaging.data() + layout->offset;
}


// マテリアル変数を設定
bool Material::SetBytes(StringId name, const void* data, uint32_t size)
{
    if(shader == nullptr) return false;

    const auto* layout = shader->findVar(name);
    if(!layout) return false;

    const uint32_t cbSize = shader->getCBPerMaterialSize();
    if(cbStaging.size() != cbSize)
    {
        // シェーダーが変わってる可能性があるので作り直す
        createConstantBuffer();
    }

    const uint32_t copySize = std::min<uint32_t>(size, layout->size);

    // 範囲チェック（layoutが壊れてる時の保険）
    if(layout->offset + copySize > cbStaging.size()) return false;

    // コピー先の値と比較して、同じならデータもdirtyフラグもそのまま
    uint8_t* dst = cbStaging.data() + layout->offset;
    if(std::memcmp(dst, data, copySize) == 0) return true;

    // データをコピー
    std::memcpy(dst, data, copySize);
    dirty = true;

    return true;
}


// -----------------------------------------------------------------------------
// レンダリング用にデバイスへ設定
// -----------------------------------------------------------------------------
bool Material::bind()
{
    // レンダリングモードが合わない場合は何もしない
    // レンダーキューを実装していない代わりの実装
    if (renderingMode != D3DManager::getInstance()->getCurrentRenderingMode())
    {
        return false;
    }

    shader->setToContext();
    for (auto& tex : textures)
    {
        if (tex != nullptr)
        {
            tex->bind();
        }
    }

    // デプス
    D3DManager::getInstance()->GetContext()->OMSetDepthStencilState(depthStencilState.Get(), 1);

    // ブレンド
    D3DManager::getInstance()->GetContext()->OMSetBlendState(blendState.Get(), NULL, 0xffffffff);

    // ラスタライザステート
    D3DManager::getInstance()->GetContext()->RSSetState(rasterizerState.Get());

    // 定数バッファ更新
    if(cbStaging.size() != shader->getCBPerMaterialSize())
    {
        createConstantBuffer();
    }

    // カラーを設定
    SetColor(StringId::intern("baseColor"), color);

    if(dirty)
    {
        if(cbStaging.size() > 0)
        {
            D3DManager::getInstance()->GetContext()->UpdateSubresource(constantBufferPerMaterial.Get(), 0, nullptr, cbStaging.data(), 0, 0);
        }
        dirty = false;
    }

    ID3D11Buffer* cbs[1] = { constantBufferPerMaterial.Get() };
    D3DManager::getInstance()->GetContext()->VSSetConstantBuffers(CB_PerMaterial, 1, cbs);
    D3DManager::getInstance()->GetContext()->PSSetConstantBuffers(CB_PerMaterial, 1, cbs);

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
    if(cbStaging.size() != shader->getCBPerMaterialSize())
    {
        createConstantBuffer();
    }

    // ラスタライザステート
    D3D11_RASTERIZER_DESC desc = {};
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = cullMode;
    D3DManager::getInstance()->GetDevice()->CreateRasterizerState(&desc, &rasterizerState);
}


void Material::createConstantBuffer()
{
    if(shader == nullptr) return;

    cbStaging.assign(shader->getCBPerMaterialSize(), 0);

    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth = shader->getCBPerMaterialSize();
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    D3DManager::getInstance()->GetDevice()->CreateBuffer(&desc, nullptr, constantBufferPerMaterial.GetAddressOf());

    dirty = true;
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
