#include "pch.h"
#include <UniDx/GltfModel.h>

#include <tiny_gltf.h>
#include <codecvt>
#include <algorithm>


namespace UniDx{

using namespace std;

namespace {

static const Matrix4x4 xFlipMtx = Matrix4x4(
    -1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f
);
const unsigned char* getAccessorData(const tinygltf::Model& model, const tinygltf::Accessor& accessor,
    size_t& stride, size_t& count)
{
    if(accessor.bufferView < 0) return nullptr;
    const auto& bufferView = model.bufferViews[accessor.bufferView];
    const auto& buffer = model.buffers[bufferView.buffer];
    size_t offset = bufferView.byteOffset + accessor.byteOffset;

    count = accessor.count;
    stride = bufferView.byteStride;
    if(stride == 0)
    {
        stride = accessor.ByteStride(bufferView);
    }
    return buffer.data.data() + offset;
}

// tinygltf::Accessor から std::span<T> でデータを取得するヘルパー
template<typename T>
void ReadAccessorData(
    const tinygltf::Model& model,
    const tinygltf::Accessor& accessor,
    bool xflip,
    vector<T>& out)
{
    size_t stride, count;
    const unsigned char* data = getAccessorData(model, accessor, stride, count);
    if(data == nullptr) return;
    out.resize(count);

    // 型チェック
    if constexpr (is_same_v<T, Vector3>) {
        assert(accessor.type == TINYGLTF_TYPE_VEC3);
        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        for (size_t i = 0; i < count; ++i) {
            const float* v = reinterpret_cast<const float*>(data + i * stride);
            out[i] = Vector3(xflip ? -v[0] : v[0], v[1], v[2]);
        }
    }
    else if constexpr (is_same_v<T, Vector2>) {
        assert(accessor.type == TINYGLTF_TYPE_VEC2);
        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        for (size_t i = 0; i < count; ++i) {
            const float* v = reinterpret_cast<const float*>(data + i * stride);
            out[i] = Vector2(v[0], v[1]);
        }
    }
    else if constexpr(is_same_v<T, Vector4>) {
        assert(accessor.type == TINYGLTF_TYPE_VEC4);
        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        for(size_t i = 0; i < count; ++i) {
            const float* v = reinterpret_cast<const float*>(data + i * stride);
            out[i] = Vector4(xflip ? -v[0] : v[0], v[1], v[2], xflip ? -v[3] : v[3]);
        }
    }
    else if constexpr (is_same_v<T, Color>) {
        // glTFのCOLOR_0はfloat4またはubyte4
        if (accessor.type == TINYGLTF_TYPE_VEC3 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
            for (size_t i = 0; i < count; ++i) {
                const float* v = reinterpret_cast<const float*>(data + i * stride);
                out[i] = Color(v[0], v[1], v[2], 1.0f);
            }
        }
        else if (accessor.type == TINYGLTF_TYPE_VEC4 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
            for (size_t i = 0; i < count; ++i) {
                const float* v = reinterpret_cast<const float*>(data + i * stride);
                out[i] = Color(v[0], v[1], v[2], v[3]);
            }
        }
        else if (accessor.type == TINYGLTF_TYPE_VEC4 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
            for (size_t i = 0; i < count; ++i) {
                const uint8_t* v = reinterpret_cast<const uint8_t*>(data + i * stride);
                out[i] = Color(
                    v[0] / 255.0f, v[1] / 255.0f, v[2] / 255.0f, v[3] / 255.0f);
            }
        }
    }
}
template<typename TU8x4>
void ReadAccessorU8x4(
    const tinygltf::Model& model,
    const tinygltf::Accessor& accessor,
    vector<TU8x4>& out,
    bool isWeights)
{
    static_assert(std::tuple_size<TU8x4>::value == 4, "TU8x4 must be 4 elements");

    size_t stride, count;
    const unsigned char* data = getAccessorData(model, accessor, stride, count);
    if(data == nullptr) return;
    out.resize(count);

    assert(accessor.type == TINYGLTF_TYPE_VEC4);

    auto ClampU8 = [](int v) -> uint8_t {
        if(v < 0) return 0;
        if(v > 255) return 255;
        return static_cast<uint8_t>(v);
        };

    for(size_t i = 0; i < count; ++i)
    {
        TU8x4 v{};
        const uint8_t* p = reinterpret_cast<const uint8_t*>(data + i * stride);

        if(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
        {
            for(int k = 0; k < 4; ++k)
            {
                const uint8_t raw = p[k];
                v[k] = raw; // weights: normalized u8 でも raw はそのまま 0..255
            }
        }
        else if(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
        {
            const uint16_t* u16 = reinterpret_cast<const uint16_t*>(p);
            for(int k = 0; k < 4; ++k)
            {
                const uint16_t raw = u16[k];
                if(isWeights && accessor.normalized)
                {
                    const int w = static_cast<int>(std::lround((raw / 65535.0) * 255.0));
                    v[k] = ClampU8(w);
                }
                else
                {
                    // joints or non-normalized weights
                    v[k] = ClampU8(static_cast<int>(raw));
                }
            }
        }
        else if(isWeights && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
        {
            const float* f = reinterpret_cast<const float*>(p);
            for(int k = 0; k < 4; ++k)
            {
                const int w = static_cast<int>(std::lround(std::clamp(f[k], 0.0f, 1.0f) * 255.0f));
                v[k] = ClampU8(w);
            }
        }
        else
        {
            v = TU8x4{ 0,0,0,0 };
        }
        out[i] = v;
    }
}

}


// -----------------------------------------------------------------------------
// gltfファイルを読み込み
// -----------------------------------------------------------------------------
bool GltfModel::load_(const char* filePath, bool makeTextureMaterial, std::shared_ptr<Shader> shader)
{
    Debug::Log(filePath);

    model = make_unique<tinygltf::Model>();
    tinygltf::TinyGLTF loader;
    string err, warn;

    bool ok = loader.LoadBinaryFromFile(model.get(), &err, &warn, filePath);
    if (!warn.empty())
    {
        Debug::Log(warn);
    }
    if (!ok)
    {
        Debug::Log(err);
        return false;
    }

    // Meshの生成
    meshes.clear();
    for (const auto& gltfMesh : model->meshes)
    {
        auto mesh = make_shared<Mesh>();
        for (const auto& primitive : gltfMesh.primitives)
        {
            readPrimitive(mesh.get(), primitive);
        }
        meshes.push_back(mesh);
    }

    if (makeTextureMaterial)
    {
        // マテリアル
        for (int i = 0; i < model->materials.size(); ++i)
        {
            const auto& gltfMat = model->materials[i];
            auto material = std::make_shared<Material>();
            material->shader = shader;
            material->cullMode = gltfMat.doubleSided ? D3D11_CULL_NONE : D3D11_CULL_BACK;

            // glTF 内包テクスチャ（まずは baseColorTexture のみ対応）
            const int texIndex = gltfMat.pbrMetallicRoughness.baseColorTexture.index;
            if (texIndex >= 0)
            {
                auto& tex = textures[texIndex];
                if (tex == nullptr)
                {
                    tex = getOrCreateTextureFromGltf_(texIndex, /*isSRGB*/true);
                }
                material->AddTexture(tex);
            }
            materials[i] = material;
        }
    }

    // ノードから階層構造を作りながら姿勢を取得
    nodes.clear();
    renderer.clear();
    int sceneIndex = model->defaultScene >= 0 ? model->defaultScene : 0;
    const auto& scene = model->scenes[sceneIndex];
    for (int nodeIndex : scene.nodes)
    {
        createNodeRecursive(*model.get(), nodeIndex, gameObject, makeTextureMaterial);
    }

    // スキン情報の後処理
    for(auto& pair : skinInstance)
    {
        // スキンのジョイントに対応するノードのTransformを入れる
        auto& skin = model->skins[pair.first];
        pair.second.joints.resize(skin.joints.size(), nullptr);
        for(int i = 0; i < skin.joints.size(); ++i)
        {
            pair.second.joints[i] = nodes[skin.joints[i]];
        }

        // スキンに対する逆バインド行列を取得
        int accIndex = skin.inverseBindMatrices;
        if(0 <= accIndex && accIndex < model->accessors.size())
        {
            const tinygltf::Accessor& acc = model->accessors[accIndex];
            if(acc.type != TINYGLTF_TYPE_MAT4) continue;
            if(acc.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) continue;

            const tinygltf::BufferView& bv = model->bufferViews[acc.bufferView];
            const tinygltf::Buffer& buf = model->buffers[bv.buffer];
            const uint8_t* base = buf.data.data() + bv.byteOffset + acc.byteOffset;
            size_t stride = acc.ByteStride(bv);
            if(stride < sizeof(float) * 16) continue;

            pair.second.inverseBind.resize(acc.count);
            for(int i = 0; i < acc.count; ++i)
            {
                const uint8_t* p = base + i * stride;
                Matrix4x4 localRH;

                // glTFは列ベクトルで最初の列から、UniDxは行ベクトルで最初の行からなので、結果的に順番コピーでOK
                std::memcpy(&localRH, p, sizeof(float) * 16);

                pair.second.inverseBind[i] = xFlipMtx * localRH * xFlipMtx;
            }
        }
    }
    return true;
}


// -----------------------------------------------------------------------------
// 頂点情報を格納したプリミティブを読み取り
// -----------------------------------------------------------------------------
void GltfModel::readPrimitive(UniDx::Mesh* mesh, const tinygltf::Primitive& primitive)
{
    auto sub = make_shared<SkinnedSubMesh>();

    // POSITION
    if(auto it = primitive.attributes.find("POSITION"); it != primitive.attributes.end()) {
        const auto& accessor = model->accessors[it->second];
        sub->resizePositions(accessor.count);
        ReadAccessorData(*model, accessor, true, sub->positionsData);
    }

    // NORMAL
    if(auto it = primitive.attributes.find("NORMAL"); it != primitive.attributes.end()) {
        const auto& accessor = model->accessors[it->second];
        sub->resizeNormals(accessor.count);
        ReadAccessorData(*model, accessor, true, sub->normalsData);
    }
    // TANGENT glTF: xyz=tangent, w=bitangent sign
    if(auto it = primitive.attributes.find("TANGENT"); it != primitive.attributes.end()) {
        const auto& accessor = model->accessors[it->second];
        sub->tangentsData.resize(accessor.count);
        ReadAccessorData(*model, accessor, true, sub->tangentsData);
    }

    // COLOR_0
    if(auto it = primitive.attributes.find("COLOR_0"); it != primitive.attributes.end()) {
        const auto& accessor = model->accessors[it->second];
        sub->resizeColors(accessor.count);
        ReadAccessorData(*model, accessor, false, sub->colorsData);
    }

    // TEXCOORD_0
    if(auto it = primitive.attributes.find("TEXCOORD_0"); it != primitive.attributes.end()) {
        const auto& accessor = model->accessors[it->second];
        sub->resizeUV(accessor.count);
        ReadAccessorData(*model, accessor, false, sub->uvData);
    }
    // TEXCOORD_1
    if(auto it = primitive.attributes.find("TEXCOORD_1"); it != primitive.attributes.end()) {
        const auto& accessor = model->accessors[it->second];
        sub->resizeUV2(accessor.count);
        ReadAccessorData(*model, accessor, false, sub->uv2Data);
    }
    // TEXCOORD_2
    if(auto it = primitive.attributes.find("TEXCOORD_2"); it != primitive.attributes.end()) {
        const auto& accessor = model->accessors[it->second];
        sub->resizeUV3(accessor.count);
        ReadAccessorData(*model, accessor, false, sub->uv3Data);
    }
    // TEXCOORD_3
    if(auto it = primitive.attributes.find("TEXCOORD_3"); it != primitive.attributes.end()) {
        const auto& accessor = model->accessors[it->second];
        sub->resizeUV4(accessor.count);
        ReadAccessorData(*model, accessor, false, sub->uv4Data);
    }

    // JOINTS_0 / WEIGHTS_0 (skinning)
    if(auto it = primitive.attributes.find("JOINTS_0"); it != primitive.attributes.end()) {
        const auto& accessor = model->accessors[it->second];
        ReadAccessorU8x4(*model, accessor, sub->jointsData, /*isWeights*/false);
    }
    if(auto it = primitive.attributes.find("WEIGHTS_0"); it != primitive.attributes.end()) {
        const auto& accessor = model->accessors[it->second];
        ReadAccessorU8x4(*model, accessor, sub->weightsData, /*isWeights*/true);
    }

    // indices
    if(primitive.indices >= 0) {
        const auto& accessor = model->accessors[primitive.indices];
        sub->resizeIndices(accessor.count);
        auto& indices = sub->indicesData;

        const auto& bufferView = model->bufferViews[accessor.bufferView];
        const auto& buffer = model->buffers[bufferView.buffer];
        size_t offset = bufferView.byteOffset + accessor.byteOffset;
        const unsigned char* data = buffer.data.data() + offset;

        if(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
            // 32bit index
            memcpy(indices.data(), data, accessor.count * sizeof(uint32_t));
        }
        else if(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
            // 16bit index → 32bitへ変換
            for(size_t i = 0; i < accessor.count; ++i) {
                indices[i] = reinterpret_cast<const uint16_t*>(data)[i];
            }
        }
        else if(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
            // 8bit index → 32bitへ変換
            for(size_t i = 0; i < accessor.count; ++i) {
                indices[i] = data[i];
            }
        }

        for(size_t i = 0; i + 2 < accessor.count; i += 3)
        {
            // 座標系の反転で表裏が変わるので、インデクスを入れ替え
            std::swap(indices[i + 1], indices[i + 2]);
        }
    }

    sub->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    mesh->submesh.push_back(sub);
}


// -----------------------------------------------------------------------------
// glTF内包テクスチャを生成（baseColor用途など）
// -----------------------------------------------------------------------------
std::shared_ptr<Texture> GltfModel::getOrCreateTextureFromGltf_(int textureIndex, bool isSRGB)
{
    if (!model) return nullptr;
    if (textureIndex < 0 || textureIndex >= static_cast<int>(model->textures.size())) return nullptr;

    const tinygltf::Texture& tex = model->textures[textureIndex];
    const int sourceIndex = tex.source;
    if (sourceIndex < 0 || sourceIndex >= static_cast<int>(model->images.size())) return nullptr;

    const tinygltf::Image& img = model->images[sourceIndex];
    if (img.image.empty() || img.width <= 0 || img.height <= 0)
    {
        return nullptr;
    }

    // tinygltf::Image はデコード済みのピクセルが image に入る（多くは UNSIGNED_BYTE 8bit）
    if (img.bits != 8 || (img.pixel_type != TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE && img.pixel_type != 0))
    {
        // 16bit PNG 等が来るとここに入る可能性あり（現状は非対応）
        Debug::Log(L"[GltfModel] Embedded image format not supported (only 8bit UNORM expected).");
        return nullptr;
    }

    std::vector<uint8_t> rgba;
    const uint8_t* src = img.image.data();

    if (img.component == 4)
    {
        // RGBA
        rgba.assign(src, src + static_cast<size_t>(img.width) * img.height * 4);
    }
    else if (img.component == 3)
    {
        // RGB -> RGBA
        rgba.resize(static_cast<size_t>(img.width) * img.height * 4);
        for (int y = 0; y < img.height; ++y)
        {
            for (int x = 0; x < img.width; ++x)
            {
                const size_t si = (static_cast<size_t>(y) * img.width + x) * 3;
                const size_t di = (static_cast<size_t>(y) * img.width + x) * 4;
                rgba[di + 0] = src[si + 0];
                rgba[di + 1] = src[si + 1];
                rgba[di + 2] = src[si + 2];
                rgba[di + 3] = 255;
            }
        }
    }
    else
    {
        Debug::Log(L"[GltfModel] Embedded image component count not supported (expected 3 or 4).");
        return nullptr;
    }

    auto outTex = std::make_shared<Texture>();

    // モデル指定のラップモード
    SetAddressModeUV(outTex.get(), textureIndex);

    if (!outTex->LoadFromMemoryRGBA8(rgba.data(), img.width, img.height, isSRGB))
    {
        return nullptr;
    }
    outTex->setName(StringId::intern(img.name));

    return outTex;
}


// -----------------------------------------------------------------------------
// node生成
// -----------------------------------------------------------------------------
void GltfModel::createNodeRecursive(const tinygltf::Model& model,
    int nodeIndex,
    GameObject* parentGO, bool attachIncludeMaterial)

{
    const tinygltf::Node& node = model.nodes[nodeIndex];
    assert(parentGO);

    // GameObject を作成
    unique_ptr<GameObject> go = make_unique<GameObject>(StringId::intern(node.name));
//    Debug::Log(go->name.get());

    // 行列を取得
    Matrix4x4 localRH;
    if (!node.matrix.empty())
    {
        // 4x4行列が直接指定されている場合
        // glTFは列ベクトルで最初の列から、UniDxは行ベクトルで最初の行からなので、結果的に順番コピーでOK
        for (int i = 0; i < 16; ++i)
        {
            reinterpret_cast<float*>(&localRH)[i] = static_cast<float>(node.matrix[i]);
        }
        localRH = Matrix4x4::FromColumnMajor16(node.matrix.data());
    }
    else {
        Vector3 tRH, sRH;
        Quaternion rRH;
        tRH = node.translation.size() == 3 ? Vector3((float)node.translation[0], (float)node.translation[1], (float)node.translation[2]) : Vector3::zero;
        rRH = node.rotation.size() == 4 ? Quaternion((float)node.rotation[0], (float)node.rotation[1], (float)node.rotation[2], (float)node.rotation[3]) : Quaternion::identity;
        sRH = node.scale.size() == 3 ? Vector3((float)node.scale[0], (float)node.scale[1], (float)node.scale[2]) : Vector3::one;

        // Matrix4x4 は SimpleMath::Matrix とレイアウト互換なので代入でOK（Transform.cpp と同じ）
        localRH = Matrix4x4::Scale(sRH) * Matrix4x4::Rotate(rRH) * Matrix4x4::Translate(tRH);
    }

    // RH -> LH 変換（行ベクトル規約でも同じ「C*M*C」でOK）
    Matrix4x4 localLH = xFlipMtx * localRH * xFlipMtx;

    Vector3 position;
    Vector3 scale;
    Quaternion rotation;
    localLH.Decompose(scale, rotation, position);

    go->transform->localScale = scale;
    go->transform->localRotation = rotation;
    go->transform->localPosition = position;

    // メッシュを持っていればアタッチ
    if (node.mesh >= 0 && node.mesh < meshes.size())
    {
        // メッシュレンダラーを作成
        MeshRenderer* r;

        if(0 <= node.skin && node.skin < model.skins.size())
        {
            // スキニングメッシュ
            SkinnedMeshRenderer* sr = go->AddComponent<SkinnedMeshRenderer>();
            skinInstance[node.skin].reference.push_back(sr);
            sr->skin = &skinInstance[node.skin];
            r = sr;
        }
        else
        {
            // 固定メッシュ
            r = go->AddComponent<MeshRenderer>();
        }
        renderer.push_back(r);
        r->mesh = *meshes[node.mesh]; // メッシュのコピー

        if (attachIncludeMaterial)
        {
            // primitivesと同じ順序に生成されたサブメッシュと対応するマテリアルを設定
            r->materials.clear();
            const auto& gltfMesh = model.meshes[node.mesh];
            for (size_t primIdx = 0; primIdx < gltfMesh.primitives.size(); ++primIdx)
            {
                const auto& prim = gltfMesh.primitives[primIdx];
                const int materialIndex = prim.material; // -1 の場合はマテリアル未指定
                if (0 <= materialIndex)
                {
                    r->materials.push_back(materials[materialIndex]);
                }
                else
                {
                    r->materials.push_back(nullptr);
                }
            }
        }
    }

    // 親を設定
    GameObject* ptr = go.get();
    if (parentGO)
    {
        Transform::SetParent(move(go), parentGO->transform);
    }
    nodes[nodeIndex] = ptr->transform;

    // 子ノードを再帰
    for (int child : node.children)
    {
        createNodeRecursive(model, child, ptr, attachIncludeMaterial);
    }
}


// -----------------------------------------------------------------------------
// Textureのラップモードをこのモデルの指定インデクスのテクスチャ設定に合わせる
// -----------------------------------------------------------------------------
void GltfModel::SetAddressModeUV(Texture* texture, int texIndex) const
{
    if(model->textures.size() <= texIndex) return;

    const tinygltf::Texture& tex = model->textures[texIndex];

    int samplerIndex = tex.sampler; // -1 の場合あり
    tinygltf::Sampler sampler;
    if (samplerIndex >= 0 && samplerIndex < model->samplers.size()) {
        sampler = model->samplers[samplerIndex];
    }
    else {
        // デフォルト扱い
        sampler.wrapS = 10497; // REPEAT
        sampler.wrapT = 10497; // REPEAT
        sampler.magFilter = -1; // 未指定
        sampler.minFilter = -1; // 未指定
    }

    // 例: DirectX のアドレッシングモードへマッピング
    auto ToDXAddr = [](int wrap) {
        switch (wrap) {
        case 10497: return D3D11_TEXTURE_ADDRESS_WRAP;   // GL_REPEAT
        case 33648: return D3D11_TEXTURE_ADDRESS_MIRROR; // GL_MIRRORED_REPEAT
        case 33071: return D3D11_TEXTURE_ADDRESS_CLAMP;  // GL_CLAMP_TO_EDGE
        default:    return D3D11_TEXTURE_ADDRESS_WRAP;
        }
    };
    texture->wrapModeU = ToDXAddr(sampler.wrapS);
    texture->wrapModeV = ToDXAddr(sampler.wrapT);
}

}
