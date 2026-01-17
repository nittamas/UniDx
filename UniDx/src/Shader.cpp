#include "pch.h"
#include <UniDx/Shader.h>

#include <filesystem>
#include <d3d11.h>

#include <UniDx/D3DManager.h>
#include <UniDx/ConstantBuffer.h>

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
	D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
const std::array< D3D11_INPUT_ELEMENT_DESC, 2> VertexPC::layout =
{
	D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	D3D11_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
const std::array< D3D11_INPUT_ELEMENT_DESC, 3> VertexPTC::layout =
{
	D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	D3D11_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
const std::array< D3D11_INPUT_ELEMENT_DESC, 3> VertexPNT::layout =
{
	D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
const std::array< D3D11_INPUT_ELEMENT_DESC, 3> VertexPNC::layout =
{
	D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	D3D11_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};


bool Shader::compile(const u8string& filePath, const D3D11_INPUT_ELEMENT_DESC* layout, size_t layout_size)
{
	ID3DBlob* error = nullptr;

	// 頂点シェーダーを読み込み＆コンパイル
	ComPtr<ID3DBlob> compiledVS;
	if (FAILED(D3DCompileFromFile(ToUtf16(filePath).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_5_0", 0, 0, &compiledVS, &error)))
	{
		Debug::Log(u8"頂点シェーダーのコンパイルエラー");
		if (error)
		{
			Debug::Log(static_cast<const char*>(error->GetBufferPointer()));
		}
		return false;
	}
	// ピクセルシェーダーを読み込み＆コンパイル
	ComPtr<ID3DBlob> compiledPS;
	if (FAILED(D3DCompileFromFile(ToUtf16(filePath).c_str(), nullptr, nullptr, "PS", "ps_5_0", 0, 0, &compiledPS, &error)))
	{
		Debug::Log(u8"ピクセルシェーダーのコンパイルエラー");
		if (error)
		{
			Debug::Log(static_cast<const char*>(error->GetBufferPointer()));
		}
		return false;
	}

	// 頂点シェーダー作成
	if (FAILED(D3DManager::getInstance()->GetDevice()->CreateVertexShader(compiledVS->GetBufferPointer(), compiledVS->GetBufferSize(), nullptr, &vertex)))
	{
		Debug::Log(u8"頂点シェーダーの作成エラー");
		return false;
	}
	// ピクセルシェーダー作成
	if (FAILED(D3DManager::getInstance()->GetDevice()->CreatePixelShader(compiledPS->GetBufferPointer(), compiledPS->GetBufferSize(), nullptr, &pixel)))
	{
		Debug::Log(u8"ピクセルシェーダーの作成エラー");
		return false;
	}

	// 頂点インプットレイアウト作成
	if (FAILED(D3DManager::getInstance()->GetDevice()->CreateInputLayout(layout, (UINT)layout_size, compiledVS->GetBufferPointer(), compiledVS->GetBufferSize(), &inputLayout)))
	{
		Debug::Log(u8"頂点インプットレイアウトの作成エラー");
		return false;
	}

	std::filesystem::path path(filePath);
	fileName = StringId::intern(path.filename().u8string());
//	Debug::Log(fileName + L"は正常にコンパイルできました");

	// ピクセルシェーダーから変数のレイアウトを反映
	reflectPSLayout(compiledPS.Get());

	return true;
}

void Shader::setToContext() const
{
	D3DManager::getInstance()->GetContext()->VSSetShader(vertex.Get(), 0, 0);
	D3DManager::getInstance()->GetContext()->PSSetShader(pixel.Get(), 0, 0);
	D3DManager::getInstance()->GetContext()->IASetInputLayout(inputLayout.Get());
}

// 変数の名前を指定してレイアウトを取得
const ShaderVarLayout* Shader::findVar(StringId nameId) const
{
	auto it = std::ranges::find_if(vars, [nameId](auto& v){ return v.name == nameId; });
	return it != vars.end() ? &*it : nullptr;
}


// ピクセルシェーダーから変数のレイアウトを反映
void Shader::reflectPSLayout(ID3DBlob* psBlob)
{
	vars.clear();
	cbPerMaterialSize = 0;

	if(!psBlob) return;

	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> refl;
	if(FAILED(D3DReflect(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
		IID_ID3D11ShaderReflection, (void**)refl.GetAddressOf())))
		return;

	// PerMaterial
	ID3D11ShaderReflectionConstantBuffer* cb = refl->GetConstantBufferByName("CBPerMaterial");
	if(!cb) return;

	D3D11_SHADER_BUFFER_DESC cbd{};
	cb->GetDesc(&cbd);

	cbPerMaterialSize = (cbd.Size + 15) - (cbd.Size  + 15) % 16; // 16byte丸め
	vars.reserve(cbd.Variables);

	for(UINT i = 0; i < cbd.Variables; ++i)
	{
		auto* var = cb->GetVariableByIndex(i);
		if(!var) continue;

		D3D11_SHADER_VARIABLE_DESC vd{};
		var->GetDesc(&vd);

		ShaderVarLayout l{ StringId::intern(vd.Name), vd.StartOffset , vd.Size };
		vars.push_back(l);
	}
}

}