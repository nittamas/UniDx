/**
 * @file Component.h
 * @brief コンポーネント
 * GameObjectにアタッチして機能を追加する基本クラス
 */
#pragma once

#include <concepts>
#include <memory>

#include "Object.h"
#include "Property.h"

namespace UniDx {

// 前方宣言
class Behaviour;
class GameObject;

/**
  * @brief コンポーネントを破棄
  * 実際に削除されるタイミングはフレームの終了時
  */
void Destroy(Component* component);


/// @brief GameObjectにアタッチして機能を追加する基本クラス
class Component : public Object
{
public:
    Property<bool> enabled;
    ReadOnlyProperty<Transform*> transform;

    GameObject* gameObject = nullptr;

	bool didAwake() const { return didAwake_; }
	bool didStart() const { return didStart_; }

    // 未破棄ならAwake()を一度だけ呼び、有効なままならOnEnable()を呼ぶ
    void checkAwake()
    {
        if (didAwake_ || isCalledDestroy) return;

        Awake();
        didAwake_ = true;

        // Awake()内で破棄予約された場合、OnEnable/OnDisableは呼ばない
        if (isCalledDestroy)
        {
            _enabled = false;
            return;
        }

        // Awake()内で無効化された場合もOnEnableは呼ばない
        if (_enabled) OnEnable();
    }

    // 有効フラグが立っているかどうか確認して Start() 呼び出し
    void checkStart()
    {
        if (_enabled && didAwake_ && !didStart_)
        {
            Start();
            didStart_ = true;
        }
    }

    bool isDestroyed() const { return isCalledDestroy; }

    virtual ~Component();

    template<typename T>
    T* GetComponent() const { return gameObject->GetComponent<T>(); }

    template<typename T>
    T* GetComponentInParent() const { return gameObject->GetComponentInParent<T>(); }

protected:
    using CopyConstruct = std::unique_ptr<Component>(*)(const Component&);

    /**
      * @brief コピー構築済みのComponentをクローンとして成立させる後処理
      * @param destination 複製先Component
      *
      * 階層全体のコピー構築後、Sceneへ接続する前に呼ばれる。
      * 単純コピーで問題ないComponentは実装不要。
      * コンポーネント自体のコピーコンストラクタが必要。
      */
    virtual void CloneTo(Component& destination) const {}

    virtual void Awake() {}
    virtual void Start() {}
    virtual void OnEnable() {}
    virtual void OnDisable() {}
    virtual void OnDestroy() {}

    bool didAwake_;
    bool didStart_;
    bool isCalledDestroy;
    bool _enabled;

    Component();
    Component(const Component& source);
    void copyComponentStateFrom(const Component& source);

private:
    CopyConstruct copyConstruct_ = nullptr;

    void setEnabled(bool value);

    // コピー可能な場合にそのコンストラクタを登録する
    template<class T>
    void registerCopyConstructor()
    {
        static_assert(std::derived_from<T, Component>);

        if constexpr (std::is_copy_constructible_v<T>)
        {
            copyConstruct_ = [](const Component& source) -> std::unique_ptr<Component>
            {
                return std::make_unique<T>(static_cast<const T&>(source));
            };
        }
        else
        {
            copyConstruct_ = nullptr;
        }
    }

    [[nodiscard]] bool canCopyConstruct() const { return copyConstruct_ != nullptr; }
    [[nodiscard]] std::unique_ptr<Component> copyConstruct() const;

    void doDestroy();

    friend void Destroy(Component*);
    friend class GameObject;
};


} // namespace UniDx
