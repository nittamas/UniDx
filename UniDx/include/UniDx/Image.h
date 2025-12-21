#pragma once

#include "UIBehaviour.h"

#include <algorithm>
#include <SpriteFont.h>
#include <UniDx/Mesh.h>


namespace UniDx {

class Texture;

// --------------------
// Imageクラス
// --------------------
class Image : public UIBehaviour
{
public:
	Image();
	virtual void OnEnable() override;
	virtual void render(const Matrix4x4& proj) const override;

	std::shared_ptr<Texture> texture;
	void SetColor(Color c) { std::fill(colors.begin(), colors.end(), c); }

private:
	ComPtr<ID3D11Buffer> constantBufferPerObject;
	std::unique_ptr<SubMesh> mesh;
	std::vector<Color> colors;
};

}
