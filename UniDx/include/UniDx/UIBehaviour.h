#pragma once

#include "Behaviour.h"

namespace UniDx {

class Canvas;

// --------------------
// UIBehaviour基底クラス
// --------------------
class UIBehaviour : public Behaviour
{
public:
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void render(const Matrix4x4& proj) const {}

protected:
	Canvas* owner = nullptr;
};

}
