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

	wstring          text;
	shared_ptr<Font> font;

private:
	unique_ptr<DirectX::SpriteBatch> spriteBatch;
};

}
