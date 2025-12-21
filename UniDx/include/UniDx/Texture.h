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
    Texture() : Object([this]() {return wstring_view(fileName); }),
        wrapModeU(D3D11_TEXTURE_ADDRESS_CLAMP),
        wrapModeV(D3D11_TEXTURE_ADDRESS_CLAMP),
        m_info()
    {
    }

    // 画像ファイルを読み込む
    bool Load(const std::wstring& filePath);

    void setForRender() const;

    D3D11_TEXTURE_ADDRESS_MODE wrapModeU;
    D3D11_TEXTURE_ADDRESS_MODE wrapModeV;

protected:
    ComPtr<ID3D11SamplerState> samplerState;
    wstring fileName;

    // シェーダーリソースビュー(画像データ読み取りハンドル)
    ComPtr<ID3D11ShaderResourceView> m_srv = nullptr;

    // 画像情報
    DirectX::TexMetadata m_info;
};


} // namespace UniDx
