#include "pch.h"

#include <UniDx/UIBehaviour.h>
#include <UniDx/Canvas.h>

namespace UniDx {


void UIBehaviour::OnEnable()
{
	Behaviour::OnEnable();
	owner = GetComponentInParent<Canvas>();
	if(owner != nullptr) owner->registerUI(this);
}


void UIBehaviour::OnDisable()
{
	Behaviour::OnDisable();
	if (owner != nullptr) owner->unregisterUI(this);
}

}
