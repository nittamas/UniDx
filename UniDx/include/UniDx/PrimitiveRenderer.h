#pragma once

#include "Renderer.h"
#include "Texture.h"

namespace UniDx {

// --------------------
// CubeRendererクラス
// --------------------
class CubeRenderer : public MeshRenderer
{
public:
    template<typename TVertex>
    static std::unique_ptr<CubeRenderer> create(const std::wstring& shaderPath)
    {
        auto ptr = std::unique_ptr<CubeRenderer>(new CubeRenderer());
        ptr->AddMaterial<TVertex>(shaderPath);
        ptr->setCreateBudderType<TVertex>();
        return ptr;
    }
    template<typename TVertex>
    static std::unique_ptr<CubeRenderer> create(const std::wstring& shaderPath, const std::wstring& texturePath)
    {
        auto ptr = std::unique_ptr<CubeRenderer>(new CubeRenderer());
        ptr->AddMaterial<TVertex>(shaderPath, texturePath);
        ptr->setCreateBudderType<TVertex>();
        return ptr;
    }
    template<typename TVertex>
    static std::unique_ptr<CubeRenderer> create(std::shared_ptr<Material> material)
    {
        auto ptr = std::unique_ptr<CubeRenderer>(new CubeRenderer());
        ptr->AddMaterial(material);
        ptr->setCreateBudderType<TVertex>();
        return ptr;
    }
    template<typename TVertex>
    void setCreateBudderType()
    {
        createBufer_ = [](SubMesh* submesh) { submesh->createBuffer<TVertex>(); };
    }

protected:
    virtual void OnEnable() override;

    std::function<void(SubMesh*)> createBufer_;
};


// --------------------
// SphereRendererクラス
// --------------------
class SphereRenderer : public MeshRenderer
{
public:
    template<typename TVertex>
    static std::unique_ptr<SphereRenderer> create(const std::wstring& shaderPath)
    {
        auto ptr = std::unique_ptr<SphereRenderer>(new SphereRenderer());
        ptr->AddMaterial<TVertex>(shaderPath);
        ptr->setCreateBudderType<TVertex>();
        return ptr;
    }
    template<typename TVertex>
    static std::unique_ptr<SphereRenderer> create(const std::wstring& shaderPath, const std::wstring& texturePath)
    {
        auto ptr = std::unique_ptr<SphereRenderer>(new SphereRenderer());
        ptr->AddMaterial<TVertex>(shaderPath, texturePath);
        ptr->setCreateBudderType<TVertex>();
        return ptr;
    }
    template<typename TVertex>
    static std::unique_ptr<SphereRenderer> create(std::shared_ptr<Material> material)
    {
        auto ptr = std::unique_ptr<SphereRenderer>(new SphereRenderer());
        ptr->AddMaterial(material);
        ptr->setCreateBudderType<TVertex>();
        return ptr;
    }
    template<typename TVertex>
    void setCreateBudderType()
    {
        createBufer_ = [](SubMesh* submesh) { submesh->createBuffer<TVertex>(); };
    }

protected:
    static std::vector<Vector3> positions;
    static std::vector<Vector3> normals;
    static std::vector<Vector2> uvs;
    static std::vector<uint32_t> indices;
    static void createVertex();

    virtual void OnEnable() override;

    std::function<void(SubMesh*)> createBufer_;
};


} // namespace UniDx
