#include "pch.h"

#include <UniDx/Image.h>
#include <UniDx/D3DManager.h>
#include <UniDx/Canvas.h>
#include <UniDx/Material.h>
#include <UniDx/Shader.h>
#include <UniDx/ConstantBuffer.h>

using namespace DirectX;

namespace {

constexpr DirectX::XMFLOAT3 image_positions[] = {
    {-0.5f, -0.5f, 0}, {-0.5f,  0.5f, -0.5f}, {0.5f, -0.5f, 0}, {0.5f, 0.5f, -0.5f}
};

constexpr DirectX::XMFLOAT2 image_uvs[] = {
	{0,1}, {0,0}, {1,1}, {1,0}
};

}

namespace UniDx {


// コンストラクタ
Image::Image()
{
	mesh = make_unique<SubMesh>();
	mesh->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	colors.resize(4, Color(1, 1, 1, 1));
}


void Image::OnEnable()
{
	UIBehaviour::OnEnable();

	// 行列用の定数バッファ生成
	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = sizeof(ConstantBufferPerObject);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	D3DManager::getInstance()->GetDevice()->CreateBuffer(&desc, nullptr, constantBufferPerObject.GetAddressOf());

	mesh->positions = std::span<const Vector3>( static_cast<const Vector3*>(image_positions), std::size(image_positions));
	mesh->uv = std::span<const Vector2>(static_cast<const Vector2*>(image_uvs), std::size(image_uvs));
	mesh->colors = colors;
}


void Image::render(const Matrix4x4& proj) const
{
	UIBehaviour::render(proj);

	if (texture == nullptr)
	{
		if (mesh->vertexBuffer == nullptr)
		{
			mesh->createBuffer<VertexPC>();
		}
		owner->getDefaultMaterial()->setForRender();
	}
	else
	{
		if (mesh->vertexBuffer == nullptr)
		{
			mesh->createBuffer<VertexPTC>();
		}
		owner->getDefaultTextureMaterial()->setForRender();
	}

	// 定数バッファ
	ID3D11Buffer* cbs[1] = { constantBufferPerObject.Get() };
	D3DManager::getInstance()->GetContext()->VSSetConstantBuffers(CB_PerObject, 1, cbs);

	// ─ ワールド行列を位置に合わせて作成
	ConstantBufferPerObject cb{};
	cb.world = transform->getLocalToWorldMatrix();

	// 定数バッファ更新
	D3DManager::getInstance()->GetContext()->UpdateSubresource(constantBufferPerObject.Get(), 0, nullptr, &cb, 0, 0);

	mesh->render();
}

}
