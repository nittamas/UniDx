#pragma once

#include "UIBehaviour.h"

#include <SpriteFont.h>


namespace UniDx {

class Font;

// --------------------
// TextMeshクラス
// --------------------
class TextMesh : public UIBehaviour
{
public:
	virtual void Awake() override;
	virtual void render(const Matrix4x4& proj) const;

	u8string         text;
	shared_ptr<Font> font;
	Color            color = Color::white;

private:
	unique_ptr<DirectX::SpriteBatch> spriteBatch;
};

}
