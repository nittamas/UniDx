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
    typedef std::vector<uint8_t> Value;

    std::shared_ptr<Shader> shader;
    Color color;
    ReadOnlyProperty<Texture*> mainTexture;
    D3D11_DEPTH_WRITE_MASK depthWrite;
    D3D11_COMPARISON_FUNC ztest;
    D3D11_CULL_MODE cullMode;
    RenderingMode renderingMode;

    // コンストラクタ
    Material();

    /** @brief マテリアル変数を取得*/
    const void* GetBytes(StringId name) const;

    /** 
     * @brief マテリアル変数を設定
     * @return 値を設定したかすでにその値になっていた場合 true。書き込めなければ false
     */
    bool SetBytes(StringId name, const void* data, uint32_t size);

    int GetInt(StringId name) const { auto* p = static_cast<const int*>(GetBytes(name)); return p ? *p : 0; }
    float GetFloat(StringId name) { auto* p = static_cast<const float*>(GetBytes(name)); return p ? *p : 0.0f; }
    Color GetColor(StringId name) { auto* p = static_cast<const Color*>(GetBytes(name)); return p ? *p : Color::black; }
    Vector4 GetVector(StringId name) { auto* p = static_cast<const Vector4*>(GetBytes(name)); return p ? *p : Vector4::zero; }
    void GetMatrix(StringId name, Matrix4x4* m) { *m = *static_cast<const Matrix4x4*>(GetBytes(name)); }
    void SetInt(StringId name, int n) { SetBytes(name, &n, sizeof(int)); }
    void SetFloat(StringId name, float f) { SetBytes(name, &f, sizeof(float)); }
    void SetColor(StringId name, Color c) { SetBytes(name, &c, sizeof(Color)); }
    void SetVector(StringId name, Vector4 v) { SetBytes(name, &v, sizeof(Vector4)); }
    void SetVector(StringId name, Vector3 v) { SetVector(name, Vector4(v, 0.0f)); }
    void SetVector(StringId name, Vector2 v) { SetVector(name, Vector4(v, 0.0f, 0.0f)); }
    void SetMatrix(StringId name, const Matrix4x4& m) { SetBytes(name, &m, sizeof(Matrix4x4)); }

    // マテリアル情報設定。Render()内で呼び出す
    virtual bool bind();

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
    ComPtr<ID3D11RasterizerState> rasterizerState;

    std::vector<std::shared_ptr<Texture>> textures;
    BlendMode blendMode;

    std::vector<uint8_t> cbStaging; // GPUへ転送するマテリアル変数
    bool dirty = true;

    void createConstantBuffer();

};


} // namespace UniDx
