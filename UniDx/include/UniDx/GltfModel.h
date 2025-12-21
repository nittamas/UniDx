#pragma once

#include <tiny_gltf.h>

#include "Renderer.h"


namespace UniDx {

// --------------------
// GltfModelクラス
// --------------------
class GltfModel : public Component
{
public:
    const std::vector<std::shared_ptr<Material>>& GetMaterials() { return materials; }

    // glTF形式のモデルファイルを読み込む（モデル、シェーダ、テクスチャ1枚を指定）
    // 内部で階層構造を構築するので、あらかじめ GameObject にアタッチしておく必要がある
    template<typename TVertex>
    bool Load(const std::wstring& modelPath, const std::wstring& shaderPath, std::shared_ptr<Texture> texture)
    {
        // モデル
        if (!Load<TVertex>(modelPath)) return false;

        // マテリアルとシェーダー
        auto material = std::make_shared<Material>();
        if (!material->shader.compile<TVertex>(shaderPath)) return false;

        // テクスチャ
        SetAddressModeUV(texture.get(), 0);     // モデルで指定されたラップモード
        material->AddTexture(texture);

        AddMaterial(material);
        return true;
    }
    template<typename TVertex>
    bool Load(const std::wstring& modelPath, const std::wstring& shaderPath, const std::wstring& texturePath)
    {
        // テクスチャ読み込み
        auto tex = std::make_shared<Texture>();
        if (!tex->Load(texturePath)) return false;

        return Load<TVertex>(modelPath, shaderPath, tex);
    }

    // glTF形式のモデルファイルを読み込む（モデル、シェーダを指定）
    template<typename TVertex>
    bool Load(const std::wstring& modelPath, const std::wstring& shaderPath)
    {
        // モデル
        if (!Load<TVertex>(modelPath)) return false;

        // マテリアルとシェーダー
        auto material = std::make_shared<Material>();
        if (!material->shader.compile<TVertex>(shaderPath)) return false;

        AddMaterial(material);
        return true;
    }

    // glTF形式のモデルファイルを読み込む
    template<typename TVertex>
    bool Load(const std::wstring& filePath)
    {
        if (load_(filePath))
        {
            for (auto& sub : submesh)
            {
                sub->createBuffer<TVertex>();
            }
            return true;
        }
        return false;
    }

    // 生成した全ての Renderer にマテリアルを追加
    void AddMaterial(std::shared_ptr<Material> material)
    {
        for (auto& r : renderer)
        {
            r->AddMaterial(material);
        }
        materials.push_back(material);
    }

    // Textureのラップモードをこのモデルの指定インデクスのテクスチャ設定に合わせる
    void SetAddressModeUV(Texture* texture, int texIndex) const;

protected:
    std::vector<MeshRenderer*> renderer;
    std::vector<std::shared_ptr<Material>> materials;
    std::unique_ptr< tinygltf::Model> model;
    std::vector< std::shared_ptr<SubMesh> > submesh;

    bool load_(const std::wstring& filePath);
    void createNodeRecursive(const tinygltf::Model& model, int nodeIndex, GameObject* parentGO);
};


} // namespace UniDx
