#pragma once

#include "GameObject.h"
#include "Transform.h"


namespace UniDx
{

template<typename... ComponentPtrs>
GameObject::GameObject(const std::wstring& name, Vector3 position, ComponentPtrs&&... components) : GameObject(name)
{
    transform->position = position;
    Add(std::forward<ComponentPtrs>(components)...);
}

} // namespace UniDx
