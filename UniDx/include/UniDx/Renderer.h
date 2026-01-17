/**
 * @file Renderer.h
 * @brief 3D描画を行うコンポーネント
 */
#pragma once

#include <memory>
#include <map>

#include "Component.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "ConstantBuffer.h"


namespace UniDx {

class Camera;
class Material;


 /// @brief 3D描画を行う基本コンポーネント
class Renderer : public Component
{
public:
    std::vector< std::shared_ptr<Material> > materials;
    int lightCount = 0;

    virtual void render(const Camera& camera) {}

    /** @brief マテリアルを追加（共有） */
    void AddMaterial(std::shared_ptr<Material> material)
    {
        materials.push_back(material);
    }

    /** @brief  シェーダー読み込んでマテリアルを追加 */
    template<typename TVertex>
    void AddMaterial(const u8string& shaderPath)
    {
        // マテリアルを追加
        materials.push_back(std::make_shared<Material>());

        // マテリアルをシェーダーを読み込んで初期化
        materials.back()->shader->compile<TVertex>(shaderPath);
    }

    /** @brief シェーダーとテクスチャを読み込んでマテリアルを追加 */
    template<typename TVertex>
    void AddMaterial(const u8string& shaderPath, const u8string& textuePath)
    {
        AddMaterial<TVertex>(shaderPath);

        // テクスチャを読み込んでマテリアルに追加
        std::unique_ptr<Texture> t = std::make_unique<Texture>();
        t->Load(textuePath);
        materials.back()->AddTexture(std::move(t));
    }

protected:
    ComPtr<ID3D11Buffer> constantBufferPerObject;

    virtual void OnEnable() override;
    virtual void createConstantBufferPerObject();
    virtual void bindPerObject();
    virtual void bindLightPerObject();
};

/// @brief メッシュ用のレンダラーコンポーネント
class MeshRenderer : public Renderer
{
public:
    Mesh mesh;

    MeshRenderer();

    // メッシュを使って描画
    virtual void render(const Camera& camera) override;
};


} // namespace UniDx
