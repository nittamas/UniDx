#include "pch.h"
#include <UniDx/Texture.h>

#include <filesystem>

#include <UniDx/D3DManager.h>


namespace UniDx
{

bool Texture::Load(const std::wstring& filePath)
{
	// WIC画像を読み込む
	auto image = std::make_unique<DirectX::ScratchImage>();
	if (FAILED(DirectX::LoadFromWICFile(filePath.c_str(), DirectX::WIC_FLAGS_NONE, &m_info, *image)))
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
	fileName = path.filename();

	// サンプラ
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = wrapModeU;
	samplerDesc.AddressV = wrapModeV;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;

	ID3D11SamplerState* pState;
	D3DManager::getInstance()->GetDevice()->CreateSamplerState(&samplerDesc, &pState);
	samplerState = pState;

	// 成功！
	return true;
}


void Texture::setForRender() const
{
	// テクスチャのバインド
	D3DManager::getInstance()->GetContext()->PSSetShaderResources(UNIDX_PS_SLOT_ALBEDO, 1, m_srv.GetAddressOf());

	// サンプラのバインド
	ID3D11SamplerState* pState = samplerState.Get();
	D3DManager::getInstance()->GetContext()->PSSetSamplers(UNIDX_PS_SLOT_ALBEDO, 1, &pState);
}

}