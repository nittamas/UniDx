#pragma once

#include <memory>
#include <DirectXTex.h>

#include "Component.h"
#include "Shader.h"


namespace UniDx {

class Camera;

// --------------------
// Textureクラス
// --------------------
class Texture : public Object
{
public:
    D3D11_TEXTURE_ADDRESS_MODE wrapModeU;
    D3D11_TEXTURE_ADDRESS_MODE wrapModeV;

    Texture() : Object([this]() {return fileName; }),
        wrapModeU(D3D11_TEXTURE_ADDRESS_CLAMP),
        wrapModeV(D3D11_TEXTURE_ADDRESS_CLAMP),
        m_info()
    {
    }

    /** @brief 画像ファイルから読み込む*/
    bool Load(const u8string& filePath);

    /**
     * @brief メモリ上のRGBA8画像(UNORM)からテクスチャを生成する
     * @param pixels: width * height * 4 bytes
     */
    bool LoadFromMemoryRGBA8(const void* pixels, int width, int height, bool isSRGB);

    void bind() const;

    void setName(StringId n) { fileName = n; }

protected:
    ComPtr<ID3D11SamplerState> samplerState;
    StringId fileName;

    // シェーダーリソースビュー(画像データ読み取りハンドル)
    ComPtr<ID3D11ShaderResourceView> m_srv = nullptr;

    // 画像情報
    DirectX::TexMetadata m_info;

    void ensureSampler_();
};


} // namespace UniDx
