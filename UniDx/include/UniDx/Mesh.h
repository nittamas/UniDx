#pragma once

#include <memory>
#include <span>
#include <vector>

#include <d3d11.h>

#include "Object.h"
#include "Property.h"
#include "Shader.h"


namespace UniDx {

class Camera;
class Texture;
class Material;

// --------------------
// SubMesh構造体
// --------------------
struct SubMesh
{
    D3D11_PRIMITIVE_TOPOLOGY topology;

    std::span<const Vector3> positions;
    std::span<const Vector3> normals;
    std::span<const Vector4> tangents;
    std::span<const Color> colors;
    std::span<const Vector2> uv;
    std::span<const Vector2> uv1;
    std::span<const Vector2> uv2;
    std::span<const Vector2> uv3;
    std::span<const uint32_t> indices;

    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;

    UINT stride;

    template<typename TVertex>
    size_t copyTo(std::span<TVertex> vertex)
    {
        assert(vertex.size() >= positions.size());

        // 位置のコピー
        for (int i = 0; i < positions.size(); ++i)
        {
            vertex[i].setPosition(positions[i]);
        }
        // 法線のコピー
        copyNormalTo(vertex);

        // 接線のコピー
        copyTangentTo(vertex);

        // カラーのコピー
        copyColorTo(vertex);

        // uvのコピー
        copyUVTo(vertex);
        copyUV1To(vertex);
        copyUV2To(vertex);
        copyUV3To(vertex);

        return positions.size();
    }

    // ID3D11Bufferの頂点バッファとインデックスバッファを作成
    // 戻ったバッファは破棄しても良い（DirectX12では仕様が変わる）
    template<typename TVertex>
    std::unique_ptr< std::vector<TVertex> > createBuffer()
    {
        // メモリ上に頂点を確保
        std::unique_ptr < std::vector<TVertex> > buf = std::make_unique< std::vector<TVertex> >();
        buf->resize(positions.size());

        // 確保したメモリに各属性データをコピー
        copyTo(std::span<TVertex>(*buf));

        // ID3D11Buffer を作成
        stride = sizeof(TVertex);
        createVertexBuffer(&buf->front());

        // インデックスが設定されていればバッファを作成
        if (indices.size() > 0)
        {
            createIndexBuffer();
        }
        // メモリ上のデータを返す。DirextX11では即座に開放して良い
        return buf;
    }
    template<typename TVertex, typename F>
    std::unique_ptr< std::vector<TVertex> > createBuffer(F func)
    {
        // メモリ上に頂点を確保
        std::unique_ptr < std::vector<TVertex> > buf = std::make_unique< std::vector<TVertex> >();
        buf->resize(positions.size());

        // 確保したメモリに各属性データをコピー
        copyTo(std::span<TVertex>(*buf));
        func(std::span<TVertex>(*buf));

        // ID3D11Buffer を作成
        stride = sizeof(TVertex);
        createVertexBuffer(&buf->front());

        // インデックスが設定されていればバッファを作成
        if (indices.size() > 0)
        {
            createIndexBuffer();
        }
        // メモリ上のデータを返す。DirextX11では即座に開放して良い
        return buf;
    }

    // GPUにバッファを作成
    void createVertexBuffer(void* data);
    void createIndexBuffer();

    // 描画
    void render() const;

    // 法線のコピー
    template<typename TVertex>
    void copyNormalTo(std::span<TVertex> vertex)
    {
        if(normals.size() == 0) return;
        assert(normals.size() == positions.size());
        for (int i = 0; i < positions.size(); ++i) vertex[i].setNormal(normals[i]);
    }

    // 接線のコピー
    template<typename TVertex>
    void copyTangentTo(std::span<TVertex> vertex)
    {
        if(tangents.size() == 0) return;
        assert(tangents.size() == positions.size());
        for(int i = 0; i < positions.size(); ++i) vertex[i].setTangent(tangents[i]);
    }

    // カラーのコピー
    template<typename TVertex>
    void copyColorTo(std::span<TVertex> vertex)
    {
        if(colors.size() == 0) return;
        assert(colors.size() == positions.size());
        for (int i = 0; i < positions.size(); ++i) vertex[i].setColor(colors[i]);
    }

    // UVのコピー
    template<typename TVertex>
    void copyUVTo(std::span<TVertex> vertex)
    {
        if(uv.size() == 0) return;
        assert(uv.size() == positions.size());
        for (int i = 0; i < positions.size(); ++i) vertex[i].setUV(uv[i]);
    }
    template<typename TVertex>
    void copyUV1To(std::span<TVertex> vertex)
    {
        if(uv1.size() == 0) return;
        assert(uv1.size() == positions.size());
        for(int i = 0; i < positions.size(); ++i) vertex[i].setUV1(uv1[i]);
    }
    template<typename TVertex>
    void copyUV2To(std::span<TVertex> vertex)
    {
        if(uv2.size() == 0) return;
        assert(uv2.size() == positions.size());
        for (int i = 0; i < positions.size(); ++i) vertex[i].setUV2(uv2[i]);
    }
    template<typename TVertex>
    void copyUV3To(std::span<TVertex> vertex)
    {
        if(uv3.size() == 0) return;
        assert(uv3.size() == positions.size());
        for (int i = 0; i < positions.size(); ++i) vertex[i].setUV3(uv3[i]);
    }
};


// --------------------
// OwnedSubMesh
// --------------------
struct OwnedSubMesh : public SubMesh
{
    std::vector<Vector3> positionsData;
    std::vector<Vector3> normalsData;
    std::vector<Vector4> tangentsData;
    std::vector<Color> colorsData;
    std::vector<Vector2> uvData;
    std::vector<Vector2> uv2Data;
    std::vector<Vector2> uv3Data;
    std::vector<Vector2> uv4Data;
    std::vector<uint32_t> indicesData;

    // 必要なサイズだけ確保し、spanを設定
    void resizePositions(size_t n) {
        positionsData.resize(n);
        positions = positionsData;
    }
    void resizeNormals(size_t n) {
        normalsData.resize(n);
        normals = normalsData;
    }
    void resizeTangents(size_t n) {
        tangentsData.resize(n);
        tangents = tangentsData;
    }
    void resizeColors(size_t n) {
        colorsData.resize(n);
        colors = colorsData;
    }
    void resizeUV(size_t n) {
        uvData.resize(n);
        uv = uvData;
    }
    void resizeUV2(size_t n) {
        uv2Data.resize(n);
        uv1 = uv2Data;
    }
    void resizeUV3(size_t n) {
        uv3Data.resize(n);
        uv2 = uv3Data;
    }
    void resizeUV4(size_t n) {
        uv4Data.resize(n);
        uv3 = uv4Data;
    }
    void resizeIndices(size_t n) {
        indicesData.resize(n);
        indices = indicesData;
    }
};


// --------------------
// Meshクラス
// --------------------
class Mesh : public Object
{
public:
    std::vector< std::shared_ptr<SubMesh> > submesh;

    Mesh() : Object([this]() {return name_;}) {}
    virtual ~Mesh() {}

    void render() const
    {
        for (auto& sub : submesh)
        {
            sub->render();
        }
    }

    void render(std::span<const std::shared_ptr<Material> > materials) const;
    
protected:
    StringId name_;
};




} // namespace UniDx
