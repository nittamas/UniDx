#pragma once

#include <memory>
#include <map>

#include "Component.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"

namespace UniDx {

class Camera;
class Texture;
enum RenderingMode;


// Unity のシェーダーグラフに合わせたブレンドモード
enum BlendMode
{
    BlendMode_Opaque,
    BlendMode_Alpha,
    BlendMode_PremultipliedAlpha,
    BlendMode_Additive,
};


// --------------------
// Materialクラス
// --------------------
class Material : public Object
{
public:
    std::shared_ptr<Shader> shader;
    Color color;
    ReadOnlyProperty<Texture*> mainTexture;
    D3D11_DEPTH_WRITE_MASK depthWrite;
    D3D11_COMPARISON_FUNC ztest;
    RenderingMode renderingMode;

    // コンストラクタ
    Material();

    // マテリアル情報設定。Render()内で呼び出す
    virtual bool bind() const;

    // テクスチャの取得
    std::span<std::shared_ptr<Texture>> getTextures() { return textures; }

    // テクスチャ追加
    void AddTexture(std::shared_ptr<Texture> tex);

    // ブレンドモード
    BlendMode getBlendMode() const { return blendMode; }
    void setBlendMode(BlendMode e);

    // 有効化
    virtual void OnEnable();

protected:
    ComPtr<ID3D11Buffer> constantBufferPerMaterial;
    ComPtr<ID3D11DepthStencilState> depthStencilState;
    ComPtr<ID3D11BlendState> blendState;

    std::vector<std::shared_ptr<Texture>> textures;
    BlendMode blendMode;
};


} // namespace UniDx
