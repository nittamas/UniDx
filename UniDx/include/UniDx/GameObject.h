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
#include "Scene.h"

namespace UniDx {


// 前方宣言
class Component;
class Transform;
class Collider;

/// @brief GameObjectを破棄
void Destroy(GameObject* component);

/**
  * @brief GameObjectと子孫を複製し、指定した親へ接続
  * @return 複製したGameObject。複製できないComponentがある場合はnullptr
  */
GameObject* Instantiate(const GameObject& original, Transform* parent);

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
        // デフォルトでTransformを追加。即時Awakeしないattach版を使う
        transform = attachComponent<Transform>();
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
        using ComponentType = typename std::remove_cvref_t<First>::element_type;
        static_assert(std::is_base_of_v<Component, ComponentType>, "First must own a Component");

        first->template registerCopyConstructor<ComponentType>();
        first->gameObject = this;
        Component* added = first.get();
        components.push_back(std::move(first));

        // アクティブシーンに接続済みなら、その場でAwake()/OnEnable()を呼ぶ
        if (IsConnectedToActiveScene(this)) added->checkAwake();

        Add(std::forward<Rest>(rest)...);
    }

    /// @brief コンポーネントを生成してアタッチする
    /// GameObjectがアクティブシーンに接続済みなら、その場で Awake() / OnEnable() が呼ばれる
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        T* ptr = attachComponent<T>(std::forward<Args>(args)...);

        // アクティブシーンに接続済みなら、その場でAwake()/OnEnable()を呼ぶ
        if (IsConnectedToActiveScene(this)) ptr->checkAwake();

        return ptr;
    }

    /// @brief GameObjectとその子孫について、未呼び出しのAwake()/OnEnable()を呼ぶ
    void checkAwake();

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
    /// @brief コンポーネントを生成してアタッチするだけで Awake() は呼ばない
    template<typename T, typename... Args>
    T* attachComponent(Args&&... args) {
        static_assert(std::is_base_of_v<Component, T>, "T must be a Component");
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        comp->template registerCopyConstructor<T>();
        comp->gameObject = this;
        T* ptr = comp.get();
        components.push_back(std::move(comp));
        return ptr;
    }

    StringId name_;
    std::vector<std::unique_ptr<Component>> components;
    bool isCalledDestroy = false;

private:
    // 階層とライフサイクルを壊す直接コピーを禁止。Instantiateを通す
    GameObject(const GameObject& source);
    GameObject& operator=(const GameObject&) = delete;

    [[nodiscard]] bool canInstantiate() const;
    void cloneTo(GameObject& destination) const;

    friend void Destroy(GameObject*);
    friend GameObject* Instantiate(const GameObject& original, Transform* parent);
};

} // namespace UniDx
