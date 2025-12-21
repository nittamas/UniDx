#pragma once

#include "Object.h"
#include "Property.h"

namespace UniDx {

// 前方宣言
class Behaviour;
class GameObject;


// --------------------
// Component基底クラス
// --------------------
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

    virtual ~Component();

protected:
    virtual void Awake() {}
    virtual void Start() {}
    virtual void OnEnable() {}
    virtual void OnDisable() {}
    virtual void OnDestroy() {}

    bool isCalledAwake;
    bool isCalledStart;
    bool _enabled;

    Component();
};


} // namespace UniDx
