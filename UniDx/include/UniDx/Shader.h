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
	void setTangent(Vector4 v) {}
	void setColor(Color c) {}
	void setUV(Vector2 v) {}
	void setUV1(Vector2 v) {}
	void setUV2(Vector2 v) {}
	void setUV3(Vector2 v) {}

	static const std::array< D3D11_INPUT_ELEMENT_DESC, 1> layout;
};
struct VertexPN : public VertexP
{
	Vector3 normal;
	void setNormal(Vector3 v) { normal = v; }

	static const std::array< D3D11_INPUT_ELEMENT_DESC, 2> layout;
};
struct VertexPT : public VertexP
{
	Vector2 uv0;
	void setUV(Vector2 v) { uv0 = v; }

	static const std::array< D3D11_INPUT_ELEMENT_DESC, 2> layout;
};
struct VertexPC : public VertexP
{
	Color color;
	void setColor(Color c) { color = c; }

	static const std::array< D3D11_INPUT_ELEMENT_DESC, 2> layout;
};
struct VertexPTC : public VertexPT
{
	Color color;
	void setColor(Color c) { color = c; }

	static const std::array< D3D11_INPUT_ELEMENT_DESC, 3> layout;
};
struct VertexPNT : public VertexPN
{
	Vector2 uv0;
	void setUV(Vector2 v) { uv0 = v; }

	static const std::array< D3D11_INPUT_ELEMENT_DESC, 3> layout;
};
struct VertexPNC : public VertexPN
{
	Color color;
	void setColor(Color c) { color = c; }

	static const std::array< D3D11_INPUT_ELEMENT_DESC, 3> layout;
};

// シェーダーが扱う変数のレイアウト情報
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