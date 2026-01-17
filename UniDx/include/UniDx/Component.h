/**
 * @file Component.h
 * @brief コンポーネント
 * GameObjectにアタッチして機能を追加する基本クラス
 */
#pragma once

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

    // 有効フラグが立っているかどうか確認して Awake() 呼び出し
    void checkAwake()
    {
        if (_enabled && !isCalledAwake)
        {
            Awake();
            isCalledAwake = true;

            OnEnable();
        }
    }

    // 有効フラグが立っているかどうか確認して Start() 呼び出し
    void checkStart()
    {
        if (_enabled && isCalledAwake && !isCalledStart)
        {
            Start();
            isCalledStart = true;
        }
    }

    bool isDestroyed() const { return isCalledDestroy; }

    virtual ~Component();

protected:
    virtual void Awake() {}
    virtual void Start() {}
    virtual void OnEnable() {}
    virtual void OnDisable() {}
    virtual void OnDestroy() {}

    bool isCalledAwake;
    bool isCalledStart;
    bool isCalledDestroy;
    bool _enabled;

    Component();
    void doDestroy();

    friend void Destroy(Component*);
    friend class GameObject;
};


} // namespace UniDx
