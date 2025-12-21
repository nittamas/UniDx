#pragma once

// C++のSTL
#include <string>
#include <array>

// Direct3Dの型・クラス・関数など
#include <d3d11.h>
#include <d3dcompiler.h>

// ComPtr
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include "UniDxDefine.h"
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


// ----------------------------------------------------------
// Shaderクラス
// ----------------------------------------------------------
class Shader : public Object
{
public:
	Shader() : Object([this]() {return wstring_view(fileName);}) {}

	// シェーダーのパスを指定してコンパイル
	bool compile(const std::wstring& filePath, const D3D11_INPUT_ELEMENT_DESC* layout, size_t layout_size);

	template<typename TVertex>
	bool compile(const std::wstring& filePath) { return compile(filePath, TVertex::layout.data(), TVertex::layout.size()); }

	// 描画のため、D3DDeviceContextにこのシェーダーをセット
	void setToContext() const;

protected:
	wstring fileName;

private:
	ComPtr<ID3D11VertexShader>	m_vertex = nullptr;	// 頂点シェーダー
	ComPtr<ID3D11PixelShader>	m_pixel = nullptr;	// ピクセルシェーダー
	ComPtr<ID3D11InputLayout>	m_inputLayout = nullptr;// 入力レイアウト
};

}