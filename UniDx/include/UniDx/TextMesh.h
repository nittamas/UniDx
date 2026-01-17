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
	UniDx::Property<u8string> text;
	shared_ptr<Font> font;
	Color            color = Color::white;

	TextMesh();

	const std::wstring& getU16Text() const { return u16text; }

	virtual void Awake() override;
	virtual void render(const Matrix4x4& proj) const;

private:
	unique_ptr<DirectX::SpriteBatch> spriteBatch;
	std::wstring     u16text;
};

}
