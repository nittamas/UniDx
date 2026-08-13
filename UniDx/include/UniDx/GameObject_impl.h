#pragma once

#include "GameObject.h"
#include "Transform.h"


namespace UniDx
{

template<typename... ComponentPtrs>
GameObject::GameObject(StringId name, Vector3 position, ComponentPtrs&&... components) : GameObject(name)
{
    transform->position = position;
    Add(std::forward<ComponentPtrs>(components)...);
}

template<typename... ComponentPtrs>
GameObject::GameObject(const char8_t* name, Vector3 position, ComponentPtrs&&... components) : GameObject(name)
{
    transform->position = position;
    Add(std::forward<ComponentPtrs>(components)...);
}

template<typename T>
T* GameObject::GetComponentInParent() const
{
    auto c = GetComponent<T>();
    if(c != nullptr || transform->parent == nullptr) return c;
    return transform->parent->gameObject->GetComponentInParent<T>();
}

template<typename Predicate>
GameObject* GameObject::Find(Predicate pred) const
{
    for (auto& childPtr : transform->getChildGameObjects()) {
        if (pred(childPtr.get()))
            return childPtr.get();
    }
    for (auto& childPtr : transform->getChildGameObjects()) {
        GameObject* p = childPtr->Find(pred);
        if (p != nullptr) return p;
    }
    return nullptr;
}

} // namespace UniDx
