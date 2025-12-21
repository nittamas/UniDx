#pragma once
#include <vector>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <DirectXMath.h>

#include "Object.h"
#include "Collision.h"

namespace UniDx {


// 前方宣言
class Component;
class Behaviour;
class Transform;
class Collider;


// --------------------
// GameObjectクラス
// --------------------
class GameObject : public Object
{
public:
    Transform* transform;

    const std::vector<std::unique_ptr<Component>>& GetComponents() { return components; }

    GameObject(wstring_view n = L"GameObject") : Object([this](){return wstring_view(name_);}), name_(n)
    {
        // デフォルトでTransformを追加
        transform = AddComponent<Transform>();
    }
    // 可変長引数でunique_ptr<Component>を受け取るコンストラクタ
    template<typename First, typename... ComponentPtrs>
        requires (!std::same_as<std::remove_cvref_t<First>, Vector3>)
    GameObject(wstring_view name, First&& first, ComponentPtrs&&... rest) : GameObject(name)
    {
        Add(std::forward<First>(first), std::forward<ComponentPtrs>(rest)...);
    }
    template<typename... ComponentPtrs>
    GameObject(const wstring& name, Vector3 position, ComponentPtrs&&... components);

    void Add() {} // ヘルパー関数でパック展開

    // GameObjectとそれ以降の追加
    template<typename... Rest>
    void Add(std::unique_ptr<GameObject>&& first, Rest&&... rest)
    {
        Transform::SetParent(std::move(first), transform);
        Add(std::forward<Rest>(rest)...);
    }

    // Componentとそれ以降の追加
    template<typename First, typename... Rest>
    void Add(First&& first, Rest&&... rest)
    {
        first->gameObject = this;
        components.push_back(std::move(first));
        Add(std::forward<Rest>(rest)...);
    }

    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, "T must be a Component");
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        comp->gameObject = this;
        T* ptr = comp.get();
        components.push_back(std::move(comp));
        return ptr;
    }

    template<typename T>
    T* GetComponent(bool includeInactive = false) {
        for (auto& comp : components) {
            auto casted = dynamic_cast<T*>(comp.get());
            if (casted != nullptr && (comp->enabled || includeInactive)) {
                return casted;
            }
        }
        return nullptr;
    }

    void SetName(const wstring& n) { name_ = n; }

    virtual void onTriggerEnter(Collider* other);
    virtual void onTriggerStay(Collider* other);
    virtual void onTriggerExit(Collider* other);
    virtual void onCollisionEnter(const Collision& collision);
    virtual void onCollisionStay(const Collision& collision);
    virtual void onCollisionExit(const Collision& collision);

protected:
    wstring name_;
    std::vector<std::unique_ptr<Component>> components;
};

} // namespace UniDx
