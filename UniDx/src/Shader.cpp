#include "pch.h"
#include <UniDx/Shader.h>

#include <filesystem>
#include <d3d11.h>

#include <UniDx/D3DManager.h>

#pragma comment(lib, "d3dcompiler.lib")


namespace UniDx
{
// 各頂点バッファのレイアウト
const std::array< D3D11_INPUT_ELEMENT_DESC, 1> VertexP::layout =
{
	D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
const std::array< D3D11_INPUT_ELEMENT_DESC, 2> VertexPN::layout =
{
	D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
const std::array< D3D11_INPUT_ELEMENT_DESC, 2> VertexPT::layout =
{
	D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	D3D11_INPUT_ELEMENT_DESC{ "TEXUV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
const std::array< D3D11_INPUT_ELEMENT_DESC, 2> VertexPC::layout =
{
	D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	D3D11_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
const std::array< D3D11_INPUT_ELEMENT_DESC, 3> VertexPTC::layout =
{
	D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	D3D11_INPUT_ELEMENT_DESC{ "TEXUV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	D3D11_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
const std::array< D3D11_INPUT_ELEMENT_DESC, 3> VertexPNT::layout =
{
	D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	D3D11_INPUT_ELEMENT_DESC{ "TEXUV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
const std::array< D3D11_INPUT_ELEMENT_DESC, 3> VertexPNC::layout =
{
	D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	D3D11_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};


bool Shader::compile(const std::wstring& filePath, const D3D11_INPUT_ELEMENT_DESC* layout, size_t layout_size)
{
	ID3DBlob* error = nullptr;

	// 頂点シェーダーを読み込み＆コンパイル
	ComPtr<ID3DBlob> compiledVS;
	if (FAILED(D3DCompileFromFile(filePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_5_0", 0, 0, &compiledVS, &error)))
	{
		Debug::Log(L"頂点シェーダーのコンパイルエラー");
		if (error)
		{
			Debug::Log(static_cast<const char*>(error->GetBufferPointer()));
		}
		abort();
		return false;
	}
	// ピクセルシェーダーを読み込み＆コンパイル
	ComPtr<ID3DBlob> compiledPS;
	if (FAILED(D3DCompileFromFile(filePath.c_str(), nullptr, nullptr, "PS", "ps_5_0", 0, 0, &compiledPS, &error)))
	{
		Debug::Log(L"ピクセルシェーダーシェーダーのコンパイルエラー");
		if (error)
		{
			Debug::Log(static_cast<const char*>(error->GetBufferPointer()));
		}
		return false;
	}

	// 頂点シェーダー作成
	if (FAILED(D3DManager::getInstance()->GetDevice()->CreateVertexShader(compiledVS->GetBufferPointer(), compiledVS->GetBufferSize(), nullptr, &m_vertex)))
	{
		Debug::Log(L"頂点シェーダーの作成エラー");
		return false;
	}
	// ピクセルシェーダー作成
	if (FAILED(D3DManager::getInstance()->GetDevice()->CreatePixelShader(compiledPS->GetBufferPointer(), compiledPS->GetBufferSize(), nullptr, &m_pixel)))
	{
		Debug::Log(L"ピクセルシェーダーの作成エラー");
		return false;
	}

	// 頂点インプットレイアウト作成
	if (FAILED(D3DManager::getInstance()->GetDevice()->CreateInputLayout(layout, (UINT)layout_size, compiledVS->GetBufferPointer(), compiledVS->GetBufferSize(), &m_inputLayout)))
	{
		Debug::Log(L"頂点インプットレイアウトの作成エラー");
		return false;
	}

	std::filesystem::path path(filePath);
	fileName = path.filename();
//	Debug::Log(fileName + L"は正常にコンパイルできました");

	return true;
}

void Shader::setToContext() const
{
	D3DManager::getInstance()->GetContext()->VSSetShader(m_vertex.Get(), 0, 0);
	D3DManager::getInstance()->GetContext()->PSSetShader(m_pixel.Get(), 0, 0);
	D3DManager::getInstance()->GetContext()->IASetInputLayout(m_inputLayout.Get());
}

}