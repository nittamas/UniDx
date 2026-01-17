#include "pch.h"
#include <UniDx/SkinnedMeshRenderer.h>

#include <UniDx/D3DManager.h>
#include <UniDx/Texture.h>
#include <UniDx/Material.h>

namespace UniDx{

const BoneMat3x4 BoneMat3x4::identity = BoneMat3x4(Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0));

const std::array< D3D11_INPUT_ELEMENT_DESC, 7> VertexSkin::layout =
{
    D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    D3D11_INPUT_ELEMENT_DESC{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    D3D11_INPUT_ELEMENT_DESC{ "BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    D3D11_INPUT_ELEMENT_DESC{ "BLENDWEIGHT", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};


// SkinnedMeshRendererのコンストラクタ
SkinnedMeshRenderer::SkinnedMeshRenderer()
{
    lightCount = PointLightCountMax + SpotLightCountMax;
}


// 現在の姿勢をシェーダーの定数バッファに転送
void SkinnedMeshRenderer::createConstantBufferPerObject()
{
    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth = sizeof(ConstantBufferSkinPerObject);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    D3DManager::getInstance()->GetDevice()->CreateBuffer(&desc, nullptr, constantBufferPerObject.GetAddressOf());

    constantBuffer = make_unique<ConstantBufferSkinPerObject>();
}


// 現在の姿勢をシェーダーの定数バッファに転送
void SkinnedMeshRenderer::bindPerObject()
{
    // ワールド行列を transform から合わせて作成
    constantBuffer->world = transform->localToWorldMatrix();

    // ボーン行列
    if(skin && !skin->joints.empty())
    {
        Matrix4x4 invWorld = constantBuffer->world.inverse();

        const uint32_t n = (uint32_t)std::min<size_t>(skin->joints.size(), SkinMeshBoneMax);

        for(uint32_t i = 0; i < n; ++i)
        {
            // 頂点データ → ワールド座標 → モデル座標 となる変換
            Matrix4x4 jointWorld = skin->joints[i]->localToWorldMatrix();
            Matrix4x4 m = skin->inverseBind[i] * jointWorld * invWorld;

            // CB用 3x4 に圧縮
            constantBuffer->bones[i] = BoneMat3x4::FromMatrix4x4(m);
        }
    }

    D3DManager::getInstance()->GetContext()->UpdateSubresource(constantBufferPerObject.Get(), 0, nullptr, constantBuffer.get(), 0, 0);

    // 定数バッファ更新
    ID3D11Buffer* cbs[1] = { constantBufferPerObject.Get() };
    D3DManager::getInstance()->GetContext()->VSSetConstantBuffers(CB_PerObject, 1, cbs);
}


}
