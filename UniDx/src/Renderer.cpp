#include "pch.h"
#include <UniDx/Renderer.h>

#include <UniDx/D3DManager.h>
#include <UniDx/Texture.h>
#include <UniDx/Camera.h>
#include <UniDx/Material.h>
#include <UniDx/SceneManager.h>
#include <UniDx/LightManager.h>

namespace UniDx{


// -----------------------------------------------------------------------------
// 有効化
// -----------------------------------------------------------------------------
void Renderer::OnEnable()
{
    // マテリアル有効化
    for (auto& material : materials)
    {
        material->OnEnable();
    }

    // 行列用の定数バッファ生成
    createConstantBufferPerObject();
}


// -----------------------------------------------------------------------------
// 行列用の定数バッファ生成
// -----------------------------------------------------------------------------
void Renderer::createConstantBufferPerObject()
{
    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth = sizeof(ConstantBufferPerObject);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    D3DManager::getInstance()->GetDevice()->CreateBuffer(&desc, nullptr, constantBufferPerObject.GetAddressOf());
}


// -----------------------------------------------------------------------------
// 現在の姿勢をシェーダーの定数バッファに転送
// -----------------------------------------------------------------------------
void Renderer::bindPerObject()
{
    // ワールド行列を transform から合わせて作成
    ConstantBufferPerObject cb{};
    cb.world = transform->localToWorldMatrix();
    D3DManager::getInstance()->GetContext()->UpdateSubresource(constantBufferPerObject.Get(), 0, nullptr, &cb, 0, 0);

    // 定数バッファ更新
    ID3D11Buffer* cbs[1] = { constantBufferPerObject.Get() };
    D3DManager::getInstance()->GetContext()->VSSetConstantBuffers(CB_PerObject, 1, cbs);
}


// -----------------------------------------------------------------------------
// オブジェクトに合わせたライト情報をシェーダーの定数バッファに転送
// -----------------------------------------------------------------------------
void Renderer::bindLightPerObject()
{
    if(lightCount > 0)
    {
        LightManager::getInstance()->updateLightCBufferObject(transform->position, lightCount);
    }
}


// -----------------------------------------------------------------------------
// MeshRendererのコンストラクタ
// -----------------------------------------------------------------------------
MeshRenderer::MeshRenderer()
{
    lightCount = PointLightCountMax + SpotLightCountMax;
}


// -----------------------------------------------------------------------------
// メッシュを使って描画
// -----------------------------------------------------------------------------
void MeshRenderer::render(const Camera& camera)
{
    // レンダーモードが一致するマテリアルがあるか確認
    auto it = std::ranges::find_if(materials, 
        [](auto& m){
            return m != nullptr && m->renderingMode == D3DManager::getInstance()->getCurrentRenderingMode();
        });
    if(it == materials.end())
    {
        return;
    }

    // 現在のTransformの情報をシェーダーのConstantBufferに転送
    bindPerObject();

    // オブジェクトに合わせたライト情報をシェーダーのConstantBufferに転送
    bindLightPerObject();

    //-----------------------------
    // 描画実行
    //-----------------------------
    mesh.render(materials);
}

}
