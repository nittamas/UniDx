/**
 * @file GltfModel.h
 * @brief glTF形式（.glb）のモデルデータを読み込んでレンダラーを生成する
 */
#pragma once

#include <tiny_gltf.h>

#include "SkinnedMeshRenderer.h"


namespace UniDx {

/// @brief glTF形式（.glb）のモデルデータを読み込んでレンダラーを生成するコンポーネント
class GltfModel : public Component
{
public:
    const std::unordered_map<int, std::shared_ptr<Material>>& GetMaterials() { return materials; }

    /**
     * @brief glTF形式のモデルファイルを読み込む（モデルファイル、シェーダを指定）
     * glb 内包テクスチャが存在する場合は、baseColorTexture を生成してマテリアルに設定する
     * 内部で階層構造を構築するので、あらかじめ GameObject にアタッチしておく必要がある
     */
    template<typename TVertex>
    bool Load(const u8string& modelPath, const u8string& shaderPath)
    {
        // 共有シェーダー
        auto shader = std::make_shared<Shader>();
        if (!shader->compile<TVertex>(shaderPath)) return false;

        // モデル
        if (!Load<TVertex>(modelPath, true, shader)) return false;
        return true;
    }

    /**
     * @brief glTF形式のモデルファイルを読み込む（モデル、シェーダ、テクスチャ1枚を指定）
     * 内部で階層構造を構築するので、あらかじめ GameObject にアタッチしておく必要がある
     */
    template<typename TVertex>
    bool Load(const u8string& modelPath, const u8string& shaderPath, std::shared_ptr<Texture> texture)
    {
        // モデル
        if (!Load<TVertex>(modelPath, false, nullptr)) return false;

        // マテリアルとシェーダー
        auto material = std::make_shared<Material>();
        if (!material->shader->compile<TVertex>(shaderPath)) return false;

        // テクスチャ
        SetAddressModeUV(texture.get(), 0);     // モデルで指定されたラップモード
        material->AddTexture(texture);

        AddMaterial(0, material);
        return true;
    }

    /**
     * @brief glTF形式のモデルファイルを読み込む（モデル、シェーダ、1枚のテクスチャファイルを指定）
     * 内部で階層構造を構築するので、あらかじめ GameObject にアタッチしておく必要がある
     */
    template<typename TVertex>
    bool Load(const u8string& modelPath, const u8string& shaderPath, const u8string& texturePath)
    {
        auto tex = std::make_shared<Texture>();
        if (!tex->Load(texturePath)) return false; // テクスチャ読み込み
        return Load<TVertex>(modelPath, shaderPath, tex);
    }


    /**
     * @brief glTF形式のモデルファイルを読み込む（モデルファイル、共有マテリアルを指定）
     * 内部で階層構造を構築するので、あらかじめ GameObject にアタッチしておく必要がある
     */
    template<typename TVertex>
    bool Load(const u8string& modelPath, std::shared_ptr<Material> material)
    {
        if (!Load<TVertex>(modelPath, false, nullptr)) return false;
        AddMaterial(0, material);
        return true;
    }

    // glTF形式のモデルファイルを読み込む
    template<typename TVertex>
    bool Load(const u8string& filePath, bool makeTextureMaterial, std::shared_ptr<Shader> shader)
    {
        if (!load_(reinterpret_cast<const char*>(filePath.c_str()), makeTextureMaterial, shader)) return false;
        for (auto& mesh : meshes)
        {
            for (auto& sub : mesh->submesh)
            {
                auto buf = sub->createBuffer<TVertex>(
                    // バッファを作るときにスキニング用のデータもコピー
                    [sub](auto buf)
                    { static_cast<SkinnedSubMesh&>(*sub).copySkinTo(buf); }
                );
            }
        }
        return true;
    }

    // 生成した全ての Renderer にマテリアルを追加
    void AddMaterial(int index, std::shared_ptr<Material> material)
    {
        for (auto& r : renderer)
        {
            r->AddMaterial(material);
        }
        materials[index] = material;
    }

    // Textureのラップモードをこのモデルの指定インデクスのテクスチャ設定に合わせる
    void SetAddressModeUV(Texture* texture, int texIndex) const;

protected:
    std::vector<MeshRenderer*> renderer;
    std::unordered_map<int, std::shared_ptr<Material>> materials;
    std::unique_ptr< tinygltf::Model> model;
    std::vector< std::shared_ptr<Mesh> > meshes; // model->meshesの順に従ったメッシュ
    std::unordered_map<int, std::shared_ptr<Texture>> textures;
    std::unordered_map<int, Transform*> nodes;
    std::unordered_map<int, SkinInstance> skinInstance;

    virtual bool load_(const char* filePath, bool makeTextureMaterial, std::shared_ptr<Shader> shader);
    virtual void readPrimitive(UniDx::Mesh* mesh, const tinygltf::Primitive& primitive);
    virtual void createNodeRecursive(const tinygltf::Model& model, int nodeIndex, GameObject* parentGO, bool attachIncludeMaterial);
    virtual std::shared_ptr<Texture> getOrCreateTextureFromGltf_(int textureIndex, bool isSRGB);
};


} // namespace UniDx
