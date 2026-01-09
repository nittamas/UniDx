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
    std::span<const Color> colors;
    std::span<const Vector2> uv;
    std::span<const Vector2> uv2;
    std::span<const Vector2> uv3;
    std::span<const Vector2> uv4;
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

        // カラーのコピー
        copyColorTo(vertex);

        // uvのコピー
        copyUVTo(vertex);
        copyUV2To(vertex);
        copyUV3To(vertex);
        copyUV4To(vertex);

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
    template<typename TVertex>
    void copyUV4To(std::span<TVertex> vertex)
    {
        if(uv4.size() == 0) return;
        assert(uv4.size() == positions.size());
        for (int i = 0; i < positions.size(); ++i) vertex[i].setUV4(uv4[i]);
    }
};


// --------------------
// OwnedSubMesh
// --------------------
struct OwnedSubMesh : public SubMesh
{
    const std::vector<Vector3>& mutablePositions() { return positions_data; }
    const std::vector<Vector3>& mutableNormals() { return normals_data; }
    const std::vector<Color>&   mutableColors() { return colors_data; }
    const std::vector<Vector2>& mutableUV() { return uv_data; }
    const std::vector<Vector2>& mutableUV2() { return uv2_data; }
    const std::vector<Vector2>& mutableUV3() { return uv3_data; }
    const std::vector<Vector2>& mutableUV4() { return uv4_data; }
    const std::vector<uint32_t>& mutableIndices() { return indices_data; }

    // 必要なサイズだけ確保し、spanを設定
    void resizePositions(size_t n) {
        positions_data.resize(n);
        positions = std::span<const Vector3>(positions_data.data(), n);
    }
    void resizeNormals(size_t n) {
        normals_data.resize(n);
        normals = std::span<const Vector3>(normals_data.data(), n);
    }
    void resizeColors(size_t n) {
        colors_data.resize(n);
        colors = std::span<const Color>(colors_data.data(), n);
    }
    void resizeUV(size_t n) {
        uv_data.resize(n);
        uv = std::span<const Vector2>(uv_data.data(), n);
    }
    void resizeUV2(size_t n) {
        uv2_data.resize(n);
        uv2 = std::span<const Vector2>(uv2_data.data(), n);
    }
    void resizeUV3(size_t n) {
        uv3_data.resize(n);
        uv3 = std::span<const Vector2>(uv3_data.data(), n);
    }
    void resizeUV4(size_t n) {
        uv4_data.resize(n);
        uv4 = std::span<const Vector2>(uv4_data.data(), n);
    }
    void resizeIndices(size_t n) {
        indices_data.resize(n);
        indices = std::span<const uint32_t>(indices_data.data(), n);
    }

protected:
    std::vector<Vector3> positions_data;
    std::vector<Vector3> normals_data;
    std::vector<Color> colors_data;
    std::vector<Vector2> uv_data;
    std::vector<Vector2> uv2_data;
    std::vector<Vector2> uv3_data;
    std::vector<Vector2> uv4_data;
    std::vector<uint32_t> indices_data;
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
