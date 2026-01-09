#include "pch.h"

#include <UniDx/Canvas.h>

#include <filesystem>
#include <DirectXMath.h>
#include <UniDx/PlayerLoop.h>
#include <UniDx/UIBehaviour.h>
#include <UniDx/Shader.h>
#include <UniDx/Material.h>
#include <UniDx/D3DManager.h>

namespace UniDx {


using namespace DirectX;

void Canvas::Awake()
{
	size = D3DManager::getInstance()->getScreenSize();
}


void Canvas::LoadDefaultMaterial(const char8_t* assetPath)
{
	std::filesystem::path assetRoot = assetPath;
	defaultMaterial = std::make_unique<Material>();
	defaultMaterial->shader->compile<VertexPC>( (assetRoot / "Color.hlsl").u8string());
	defaultTextureMaterial = std::make_unique<Material>();
	defaultTextureMaterial->shader->compile<VertexPTC>((assetRoot / "Sprite.hlsl").u8string());
}


void Canvas::OnEnable()
{
	Behaviour::OnEnable();
	PlayerLoop::getInstance()->registerCanvas(this);
}


void Canvas::OnDisable()
{
	Behaviour::OnDisable();
	PlayerLoop::getInstance()->unregisterCanvas(this);
}


void Canvas::registerUI(UIBehaviour* e)
{
	elements_.push_back(e);
}


void Canvas::unregisterUI(UIBehaviour* e)
{
	auto it = std::find(elements_.begin(), elements_.end(), e);
	if (it != elements_.end()) elements_.erase(it);
}


Material* Canvas::getDefaultMaterial()
{
	return defaultMaterial.get();
}


Material* Canvas::getDefaultTextureMaterial()
{
	return defaultTextureMaterial.get();
}


void Canvas::render() const
{
	Matrix4x4 proj( XMMatrixOrthographicLH(size.x, size.y, -1.0f, 1.0f) );

	for (auto& it : elements_)
	{
		it->render(proj);
	}
}

}
