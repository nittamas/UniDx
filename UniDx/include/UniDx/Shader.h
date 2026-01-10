#pragma once

#include "UniDxDefine.h"
#include "StringId.h"
#include "Math.h"
#include "Object.h"

namespace UniDx
{

// 頂点バッファ
struct VertexP
{
	Vector3 position;

	void setPosition(Vector3 v) { position = v; }
	void setNormal(Vector3 v) {}
	void setColor(Color c) {}
	void setUV(Vector2 v) {}
	void setUV2(Vector2 v) {}
	void setUV3(Vector2 v) {}
	void setUV4(Vector2 v) {}

	static const std::array< D3D11_INPUT_ELEMENT_DESC, 1> layout;
};
struct VertexPN
{
	Vector3 position;
	Vector3 normal;

	void setPosition(Vector3 v) { position = v; }
	void setNormal(Vector3 v) { normal = v; }
	void setColor(Color c) {}
	void setUV(Vector2 v) {}
	void setUV2(Vector2 v) {}
	void setUV3(Vector2 v) {}
	void setUV4(Vector2 v) {}

	static const std::array< D3D11_INPUT_ELEMENT_DESC, 2> layout;
};
struct VertexPT
{
	Vector3 position;
	Vector2 uv0;

	void setPosition(Vector3 v) { position = v; }
	void setNormal(Vector3 v) {}
	void setColor(Color c) {}
	void setUV(Vector2 v) { uv0 = v; }
	void setUV2(Vector2 v) { }
	void setUV3(Vector2 v) {}
	void setUV4(Vector2 v) {}

	static const std::array< D3D11_INPUT_ELEMENT_DESC, 2> layout;
};
struct VertexPC
{
	Vector3 position;
	Color color;

	void setPosition(Vector3 v) { position = v; }
	void setNormal(Vector3 v) {}
	void setColor(Color c) { color = c; }
	void setUV(Vector2 v) {}
	void setUV2(Vector2 v) {}
	void setUV3(Vector2 v) {}
	void setUV4(Vector2 v) {}

	static const std::array< D3D11_INPUT_ELEMENT_DESC, 2> layout;
};
struct VertexPTC
{
	Vector3 position;
	Vector2 uv0;
	Color color;

	void setPosition(Vector3 v) { position = v; }
	void setNormal(Vector3 v) {}
	void setColor(Color c) { color = c; }
	void setUV(Vector2 v) { uv0 = v; }
	void setUV2(Vector2 v) {}
	void setUV3(Vector2 v) {}
	void setUV4(Vector2 v) {}

	static const std::array< D3D11_INPUT_ELEMENT_DESC, 3> layout;
};
struct VertexPNT
{
	Vector3 position;
	Vector3 normal;
	Vector2 uv0;

	void setPosition(Vector3 v) { position = v; }
	void setNormal(Vector3 v) { normal = v; }
	void setColor(Color c) {}
	void setUV(Vector2 v) { uv0 = v; }
	void setUV2(Vector2 v) {}
	void setUV3(Vector2 v) {}
	void setUV4(Vector2 v) {}

	static const std::array< D3D11_INPUT_ELEMENT_DESC, 3> layout;
};
struct VertexPNC
{
	Vector3 position;
	Vector3 normal;
	Color color;

	void setPosition(Vector3 v) { position = v; }
	void setNormal(Vector3 v) { normal = v; }
	void setColor(Color c) { color = c; }
	void setUV(Vector2 v) {}
	void setUV2(Vector2 v) {}
	void setUV3(Vector2 v) {}
	void setUV4(Vector2 v) {}

	static const std::array< D3D11_INPUT_ELEMENT_DESC, 3> layout;
};


struct ShaderVarLayout
{
	StringId  name;
	uint32_t  offset;
	uint32_t  size;
};

/** @brief シェーダーをコンパイルしてマテリアルの変数レイアウトを保持する*/
class Shader : public Object
{
public:
	Shader() : Object([this]() {return fileName;}) {}

	bool compile(const u8string& filePath, const D3D11_INPUT_ELEMENT_DESC* layout, size_t layout_size);

	/** @brief 頂点タイプをテンプレート引数に、シェーダーのパスを引数に指定してコンパイル*/
	template<typename TVertex>
	bool compile(const u8string& filePath) { return compile(filePath, TVertex::layout.data(), TVertex::layout.size()); }

	// 描画のため、D3DDeviceContextにこのシェーダーをセット
	void setToContext() const;

	// 変数の名前を指定してレイアウトを取得
	const ShaderVarLayout* findVar(StringId nameId) const;
	const int getCBPerMaterialSize() const { return cbPerMaterialSize; }

protected:
	StringId fileName;

	// ピクセルシェーダーから変数のレイアウトを反映
	void reflectPSLayout(ID3DBlob* psBlob);

private:
	ComPtr<ID3D11VertexShader>	vertex = nullptr;	// 頂点シェーダー
	ComPtr<ID3D11PixelShader>	pixel = nullptr;	// ピクセルシェーダー
	ComPtr<ID3D11InputLayout>	inputLayout = nullptr;// 入力レイアウト

	std::vector<ShaderVarLayout> vars;
	int cbPerMaterialSize = 0;
	ComPtr<ID3D11Buffer> cbPerMaterial;
};

}