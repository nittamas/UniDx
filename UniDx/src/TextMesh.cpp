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
    Vector3 scale = transform->localScale; // 現状はローカルスケールのみ
    Vector2 drawPos(pos.x, pos.y);
    font->getSpriteFont()->DrawString(
        spriteBatch.get(), ToUtf16(text).c_str(), drawPos, color.XMLoad(), 0.0f, Vector2::zero,
        Vector2(scale.x, scale.y) );

    spriteBatch->End();
}

}
