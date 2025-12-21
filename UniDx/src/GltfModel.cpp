#include "pch.h"
#include <UniDx/GltfModel.h>

#include <tiny_gltf.h>
#include <codecvt>


namespace UniDx{

using namespace std;

namespace {

// tinygltf::Accessor から std::span<T> でデータを取得するヘルパー
template<typename T>
void ReadAccessorData(
    const tinygltf::Model& model,
    const tinygltf::Accessor& accessor,
    vector<T>& out)
{
    if (accessor.bufferView < 0) return;
    const auto& bufferView = model.bufferViews[accessor.bufferView];
    const auto& buffer = model.buffers[bufferView.buffer];
    size_t offset = bufferView.byteOffset + accessor.byteOffset;
    size_t count = accessor.count;
    const unsigned char* data = buffer.data.data() + offset;
    out.resize(count);

    // 型チェック
    if constexpr (is_same_v<T, Vector3>) {
        assert(accessor.type == TINYGLTF_TYPE_VEC3);
        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        for (size_t i = 0; i < count; ++i) {
            const float* v = reinterpret_cast<const float*>(data + i * bufferView.byteStride);
            out[i] = Vector3(v[0], v[1], v[2]);
        }
    }
    else if constexpr (is_same_v<T, Vector2>) {
        assert(accessor.type == TINYGLTF_TYPE_VEC2);
        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        for (size_t i = 0; i < count; ++i) {
            const float* v = reinterpret_cast<const float*>(data + i * bufferView.byteStride);
            out[i] = Vector2(v[0], v[1]);
        }
    }
    else if constexpr (is_same_v<T, Color>) {
        // glTFのCOLOR_0はfloat4またはubyte4
        if (accessor.type == TINYGLTF_TYPE_VEC3 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
            for (size_t i = 0; i < count; ++i) {
                const float* v = reinterpret_cast<const float*>(data + i * bufferView.byteStride);
                out[i] = Color(v[0], v[1], v[2], 1.0f);
            }
        }
        else if (accessor.type == TINYGLTF_TYPE_VEC4 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
            for (size_t i = 0; i < count; ++i) {
                const float* v = reinterpret_cast<const float*>(data + i * bufferView.byteStride);
                out[i] = Color(v[0], v[1], v[2], v[3]);
            }
        }
        else if (accessor.type == TINYGLTF_TYPE_VEC4 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
            for (size_t i = 0; i < count; ++i) {
                const uint8_t* v = reinterpret_cast<const uint8_t*>(data + i * bufferView.byteStride);
                out[i] = Color(
                    v[0] / 255.0f, v[1] / 255.0f, v[2] / 255.0f, v[3] / 255.0f);
            }
        }
    }
}

}


// -----------------------------------------------------------------------------
// node生成
// -----------------------------------------------------------------------------
void GltfModel::createNodeRecursive(const tinygltf::Model& model,
    int nodeIndex,
    GameObject* parentGO)

{
    const tinygltf::Node& node = model.nodes[nodeIndex];
    assert(parentGO);

        // GameObject を作成
    unique_ptr<GameObject> go = make_unique<GameObject>();
    go->SetName( UniDx::ToUtf16(node.name) );

    // 行列を取得
    Vector3 position;
    Vector3 scale;
    Quaternion rotation;
    if (!node.matrix.empty())
    {
        // 4x4行列が直接指定されている場合は
        // どちらも列優先なので、順番にコピー
        Matrix4x4 matrix;
        for (int i = 0; i < 16; ++i)
        {
            reinterpret_cast<float*>(&matrix)[i] = static_cast<float>(node.matrix[i]);
        }
        matrix.Decompose(scale, rotation, position);
    }
    else {
        // translation/rotation/scaleから合成
        position = node.translation.size() == 3 ? Vector3((float)node.translation[0], (float)node.translation[1], (float)node.translation[2]) : Vector3::zero;
        rotation = node.rotation.size() == 4 ? Quaternion((float)node.rotation[0], (float)node.rotation[1], (float)node.rotation[2], (float)node.rotation[3]) : Quaternion::identity;
        scale = node.scale.size() == 3 ? Vector3((float)node.scale[0], (float)node.scale[1], (float)node.scale[2]) : Vector3::one;
    }
    go->transform->localScale = scale;
    go->transform->localRotation = rotation;
    go->transform->localPosition = position;

    // メッシュを持っていればアタッチ
    if (node.mesh >= 0 && node.mesh < submesh.size())
    {
        auto* r = go->AddComponent<MeshRenderer>();
        renderer.push_back(r);
        r->mesh.submesh.push_back(submesh[node.mesh]);
    }

    // 親を設定
    GameObject* ptr = go.get();
    if (parentGO)
    {
        Transform::SetParent(move(go), parentGO->transform);
    }

    // 子ノードを再帰
    for (int child : node.children)
    {
        createNodeRecursive(model, child, ptr);
    }
}


// -----------------------------------------------------------------------------
// gltfファイルを読み込み
// -----------------------------------------------------------------------------
bool GltfModel::load_(const wstring& filePath)
{
    Debug::Log(filePath);

    model = make_unique<tinygltf::Model>();
    tinygltf::TinyGLTF loader;
    string err, warn;

    auto path = ToUtf8(filePath);

    bool ok = loader.LoadBinaryFromFile(model.get(), &err, &warn, path.c_str());
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
    submesh.clear();

    for (const auto& gltfMesh : model->meshes)
    {
        for (const auto& primitive : gltfMesh.primitives)
        {
            auto sub = make_shared<OwnedSubMesh>();

            // POSITION
            if (auto it = primitive.attributes.find("POSITION"); it != primitive.attributes.end()) {
                const auto& accessor = model->accessors[it->second];
                sub->resizePositions(accessor.count);
                ReadAccessorData(*model, accessor, const_cast<vector<Vector3>&>(sub->mutablePositions()));
            }

            // NORMAL
            if (auto it = primitive.attributes.find("NORMAL"); it != primitive.attributes.end()) {
                const auto& accessor = model->accessors[it->second];
                sub->resizeNormals(accessor.count);
                ReadAccessorData(*model, accessor, const_cast<vector<Vector3>&>(sub->mutableNormals()));
            }

            // COLOR_0
            if (auto it = primitive.attributes.find("COLOR_0"); it != primitive.attributes.end()) {
                const auto& accessor = model->accessors[it->second];
                sub->resizeColors(accessor.count);
                ReadAccessorData(*model, accessor, const_cast<vector<Color>&>(sub->mutableColors()));
            }

            // TEXCOORD_0
            if (auto it = primitive.attributes.find("TEXCOORD_0"); it != primitive.attributes.end()) {
                const auto& accessor = model->accessors[it->second];
                sub->resizeUV(accessor.count);
                ReadAccessorData(*model, accessor, const_cast<vector<Vector2>&>(sub->mutableUV()));
            }
            // TEXCOORD_1
            if (auto it = primitive.attributes.find("TEXCOORD_1"); it != primitive.attributes.end()) {
                const auto& accessor = model->accessors[it->second];
                sub->resizeUV2(accessor.count);
                ReadAccessorData(*model, accessor, const_cast<vector<Vector2>&>(sub->mutableUV2()));
            }
            // TEXCOORD_2
            if (auto it = primitive.attributes.find("TEXCOORD_2"); it != primitive.attributes.end()) {
                const auto& accessor = model->accessors[it->second];
                sub->resizeUV3(accessor.count);
                ReadAccessorData(*model, accessor, const_cast<vector<Vector2>&>(sub->mutableUV3()));
            }
            // TEXCOORD_3
            if (auto it = primitive.attributes.find("TEXCOORD_3"); it != primitive.attributes.end()) {
                const auto& accessor = model->accessors[it->second];
                sub->resizeUV4(accessor.count);
                ReadAccessorData(*model, accessor, const_cast<vector<Vector2>&>(sub->mutableUV4()));
            }

            // indices
            if (primitive.indices >= 0) {
                const auto& accessor = model->accessors[primitive.indices];
                sub->resizeIndices(accessor.count);
                auto& indices = const_cast<std::vector<uint32_t>&>(sub->mutableIndices());

                const auto& bufferView = model->bufferViews[accessor.bufferView];
                const auto& buffer = model->buffers[bufferView.buffer];
                size_t offset = bufferView.byteOffset + accessor.byteOffset;
                const unsigned char* data = buffer.data.data() + offset;

                if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    // 32bit index
                    memcpy(indices.data(), data, accessor.count * sizeof(uint32_t));
                }
                else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    // 16bit index → 32bitへ変換
                    for (size_t i = 0; i < accessor.count; ++i) {
                        indices[i] = reinterpret_cast<const uint16_t*>(data)[i];
                    }
                }
                else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    // 8bit index → 32bitへ変換
                    for (size_t i = 0; i < accessor.count; ++i) {
                        indices[i] = data[i];
                    }
                }
            }

            sub->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            submesh.push_back(sub);
        }
    }

    // ノードから階層構造を作りながら姿勢を取得
    int sceneIndex = model->defaultScene >= 0 ? model->defaultScene : 0;
    const auto& scene = model->scenes[sceneIndex];
    for (int nodeIndex : scene.nodes)
    {
        createNodeRecursive(*model.get(), nodeIndex, gameObject);
    }
    return true;
}


// -----------------------------------------------------------------------------
// Textureのラップモードをこのモデルの指定インデクスのテクスチャ設定に合わせる
// -----------------------------------------------------------------------------
void GltfModel::SetAddressModeUV(Texture* texture, int texIndex) const
{
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
