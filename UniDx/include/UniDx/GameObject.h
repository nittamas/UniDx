/**
 * @file GameObject.h
 * @brief キャラクターや背景、カメラなどの基礎となるオブジェクト
 */
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
class Transform;
class Collider;

/// @brief GameObjectを破棄
void Destroy(GameObject* component);

 /// @brief キャラクターや背景カメラなどの基礎となるオブジェクト
class GameObject : public Object
{
public:
    Transform* transform;

    const std::vector<std::unique_ptr<Component>>& GetComponents() const { return components; }

    GameObject(const char* n = "GameObject") : GameObject(StringId::intern(std::string_view(n))) {}
    GameObject(const char8_t* n) : GameObject(StringId::intern(n)) {}
    GameObject(StringId n) : Object([this](){return name_;}), name_(n), isCalledDestroy(false)
    {
        // デフォルトでTransformを追加
        transform = AddComponent<Transform>();
    }

    // 可変長引数でunique_ptr<Component>を受け取るコンストラクタ
    template<typename First, typename... ComponentPtrs>
        requires (!std::same_as<std::remove_cvref_t<First>, Vector3>)
    GameObject(StringId name, First&& first, ComponentPtrs&&... rest) : GameObject(name)
    {
        Add(std::forward<First>(first), std::forward<ComponentPtrs>(rest)...);
    }
    template<typename... ComponentPtrs>
    GameObject(StringId name, Vector3 position, ComponentPtrs&&... components);

    template<typename First, typename... ComponentPtrs>
        requires (!std::same_as<std::remove_cvref_t<First>, Vector3>)
    GameObject(const char8_t* name, First&& first, ComponentPtrs&&... rest) : GameObject(name)
    {
        Add(std::forward<First>(first), std::forward<ComponentPtrs>(rest)...);
    }
    template<typename... ComponentPtrs>
    GameObject(const char8_t* name, Vector3 position, ComponentPtrs&&... components);

    // デストラクタ
    ~GameObject();

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
        static_assert(std::is_base_of_v<Component, T>, "T must be a Component");
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        comp->gameObject = this;
        T* ptr = comp.get();
        components.push_back(std::move(comp));
        return ptr;
    }

    template<typename T>
    [[nodiscard]] T* GetComponent(bool includeInactive = false) {
        for (auto& comp : components) {
            auto casted = dynamic_cast<T*>(comp.get());
            if (casted != nullptr && (comp->enabled || includeInactive && !comp->isDestroyed())) {
                return casted;
            }
        }
        return nullptr;
    }

    template<typename Predicate>
    GameObject* Find(Predicate pred) const;

    void SetName(StringId n) { name_ = n; }
    bool checkDestroy();

    virtual void onTriggerEnter(Collider* other);
    virtual void onTriggerStay(Collider* other);
    virtual void onTriggerExit(Collider* other);
    virtual void onCollisionEnter(const Collision& collision);
    virtual void onCollisionStay(const Collision& collision);
    virtual void onCollisionExit(const Collision& collision);

protected:
    StringId name_;
    std::vector<std::unique_ptr<Component>> components;
    bool isCalledDestroy = false;

    friend void Destroy(GameObject*);
};

} // namespace UniDx
