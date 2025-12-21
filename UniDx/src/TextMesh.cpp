#include "pch.h"

#include <UniDx/TextMesh.h>
#include <UniDx/D3DManager.h>
#include <UniDx/Font.h>

using namespace DirectX;

namespace UniDx {


void TextMesh::Awake()
{
	UIBehaviour::Awake();
	spriteBatch = std::make_unique<SpriteBatch>(D3DManager::getInstance()->GetContext().Get());
}


void TextMesh::render(const Matrix4x4& proj) const
{
	UIBehaviour::render(proj);
    if(spriteBatch == nullptr || font == nullptr || font->getSpriteFont() == nullptr) return;

    // SpriteFontを使った描画
    spriteBatch->Begin();

    Vector3 pos = transform->position;
    Vector2 drawPos(pos.x, pos.y);
    font->getSpriteFont()->DrawString(spriteBatch.get(), text.c_str(), drawPos);

    spriteBatch->End();
}

}
