#pragma once

#include "Behaviour.h"

namespace UniDx {

class UIBehaviour;
class Material;

// --------------------
// Canvasクラス
// --------------------
class Canvas : public Behaviour
{
public:
	virtual void Awake() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void render() const;

	void LoadDefaultMaterial(const wchar_t* assetPath);

	void registerUI(UIBehaviour* e);
	void unregisterUI(UIBehaviour* e);

	Vector2 size;

	Material* getDefaultMaterial();
	Material* getDefaultTextureMaterial();

private:
	std::vector<UIBehaviour*> elements_;
	std::unique_ptr<Material> defaultMaterial;			// 頂点はVertexPC
	std::unique_ptr<Material> defaultTextureMaterial;	// 頂点はVertexPTC
};

}
