#pragma once

#include "Renderer.h"


namespace UniDx {

class SkinnedMeshRenderer;

constexpr int WeightMax = 4;

// スキン情報
struct SkinInstance
{
    std::vector<SkinnedMeshRenderer*> reference;
    std::vector<Transform*> joints;

    // glTF由来の不変なアセットデータ。インスタンス間で共有する。
    std::shared_ptr<const std::vector<Matrix4x4>> inverseBind;
};


struct VertexSkin : public VertexPN
{
    typedef std::array<uint8_t, WeightMax> Joints;
    typedef std::array<uint8_t, WeightMax> Weights;

    Vector4 tangent;
    Vector2 uv0;
    Vector2 uv1;
    Joints joints;
    Weights weights;

    void setTangent(Vector4 v) { tangent = v; }
    void setUV(Vector2 v) { uv0 = v; }
    void setUV1(Vector2 v) { uv1 = v; }

    static const std::array< D3D11_INPUT_ELEMENT_DESC, 7> layout;
};


struct SkinnedSubMesh : public OwnedSubMesh
{
    std::vector<VertexSkin::Joints> jointsData;
    std::vector<VertexSkin::Weights> weightsData;

    template<typename TVertex>
    size_t copySkinTo(std::span<TVertex> vertex)
    { return 0; }
    size_t copySkinTo(std::span<VertexSkin> vertex)
    {
        assert(vertex.size() >= jointsData.size());

        // ジョイントインデクスとウェイトのコピー
        for(int i = 0; i < jointsData.size(); ++i)
        {
            vertex[i].joints = jointsData[i];
            vertex[i].weights = weightsData[i];
        }
        return jointsData.size();
    }
};


/** @brief メッシュ用のレンダラーコンポーネント */
class SkinnedMeshRenderer : public MeshRenderer
{
public:
    SkinInstance* skin = nullptr;
    SkinnedMeshRenderer();
    SkinnedMeshRenderer(const SkinnedMeshRenderer& source);

protected:
    virtual void CloneTo(Component& destination) const override;
    virtual void createConstantBufferPerObject() override;
    virtual void bindPerObject() override;

    unique_ptr<ConstantBufferSkinPerObject> constantBuffer;
};


} // namespace UniDx
