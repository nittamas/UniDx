#include "pch.h"
#include <UniDx/Texture.h>

#include <filesystem>

#include <UniDx/D3DManager.h>


namespace UniDx
{

void Texture::ensureSampler_()
{
    if (!samplerState)
    {
        D3D11_SAMPLER_DESC samplerDesc{};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = wrapModeU;
        samplerDesc.AddressV = wrapModeV;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MinLOD = -FLT_MAX;
        samplerDesc.MaxLOD = FLT_MAX;

        ID3D11SamplerState* newSampler;
        if (SUCCEEDED(D3DManager::getInstance()->GetDevice()->CreateSamplerState(&samplerDesc, &newSampler)))
        {
            samplerState = newSampler;
        }
    }
}


bool Texture::Load(const u8string& filePath)
{
	// WIC画像を読み込む
	auto image = std::make_unique<DirectX::ScratchImage>();
	if (FAILED(DirectX::LoadFromWICFile(ToUtf16(filePath).c_str(), DirectX::WIC_FLAGS_NONE, &m_info, *image)))
	{
		// 失敗
		m_info = {};
		return false;
	}

	// ミップマップの生成
	if (m_info.mipLevels == 1)
	{
		auto mipChain = std::make_unique<DirectX::ScratchImage>();
		if (SUCCEEDED(DirectX::GenerateMipMaps(image->GetImages(), image->GetImageCount(), image->GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, *mipChain)))
		{
			image = std::move(mipChain);
		}
	}

	// リソースとシェーダーリソースビューを作成
	if (FAILED(DirectX::CreateShaderResourceView(D3DManager::getInstance()->GetDevice().Get(), image->GetImages(), image->GetImageCount(), m_info, &m_srv)))
	{
		// 失敗
		m_info = {};
		return false;
	}

	std::filesystem::path path(filePath);
	fileName = StringId::intern(path.filename().u8string());

	// サンプラ
    ensureSampler_();

	// 成功！
	return true;
}


bool Texture::LoadFromMemoryRGBA8(const void* pixels, int width, int height, bool isSRGB)
{
    if (!pixels || width <= 0 || height <= 0)
    {
        m_info = {};
        m_srv.Reset();
        return false;
    }

    const DXGI_FORMAT fmt = isSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

    // ScratchImage を作ってコピー
    DirectX::ScratchImage image;
    if (FAILED(image.Initialize2D(fmt, static_cast<size_t>(width), static_cast<size_t>(height), 1, 1)))
    {
        m_info = {};
        return false;
    }
    auto* img = image.GetImage(0, 0, 0);
    if (!img || !img->pixels)
    {
        m_info = {};
        return false;
    }

    const uint8_t* src = reinterpret_cast<const uint8_t*>(pixels);
    for (int y = 0; y < height; ++y)
    {
        memcpy(img->pixels + img->rowPitch * y, src + static_cast<size_t>(width) * 4 * y, static_cast<size_t>(width) * 4);
    }

    m_info = image.GetMetadata();

    // ミップマップ生成
    DirectX::ScratchImage mipChain;
    if (SUCCEEDED(DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipChain)))
    {
        m_info = mipChain.GetMetadata();
        // SRV作成
        if (FAILED(DirectX::CreateShaderResourceView(D3DManager::getInstance()->GetDevice().Get(), mipChain.GetImages(), mipChain.GetImageCount(), m_info, &m_srv)))
        {
            m_info = {};
            return false;
        }
    }
    else
    {
        // ミップ生成できない場合はそのまま
        if (FAILED(DirectX::CreateShaderResourceView(D3DManager::getInstance()->GetDevice().Get(), image.GetImages(), image.GetImageCount(), m_info, &m_srv)))
        {
            m_info = {};
            return false;
        }
    }

    ensureSampler_();
    return true;
}


void Texture::bind() const
{
	// テクスチャのバインド
	D3DManager::getInstance()->GetContext()->PSSetShaderResources(UNIDX_PS_SLOT_ALBEDO, 1, m_srv.GetAddressOf());

	// サンプラのバインド
	ID3D11SamplerState* pState = samplerState.Get();
	D3DManager::getInstance()->GetContext()->PSSetSamplers(UNIDX_PS_SLOT_ALBEDO, 1, &pState);
}

}