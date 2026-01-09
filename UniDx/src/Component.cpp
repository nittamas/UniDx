#include "pch.h"
#include <UniDx/Component.h>

namespace UniDx{

// コンストラクタ
Component::Component() :
    Object([this]() { return gameObject != nullptr ? gameObject->name : StringId(); }),
    enabled(
        // get
        [this]() { return _enabled && isCalledAwake; },

        // set
        [this](bool value) {
            if (!_enabled && value && !isCalledDestroy) {
                _enabled = true;
                if (!isCalledAwake) { Awake(); isCalledAwake = true; }
                OnEnable();
            }
            else if (_enabled && !value) {
                _enabled = false;
                if (isCalledAwake) { OnDisable(); }
            }
        }
    ),
    transform(
        [this]() { return gameObject->transform; }
    ),
    _enabled(true),
    isCalledAwake(false),
    isCalledStart(false),
    isCalledDestroy(false)
{

}

void Component::doDestroy()
{
    isCalledDestroy = true; // 以降で enabled=true は無効
    if (_enabled)
    {
        enabled = false; // 無効化（この中でOnDisable()が呼ばれる）
    }
    if (isCalledAwake)
    {
        OnDestroy();
    }
}

// デストラクタ（仮想 OnDestroy をここで呼ばない）
Component::~Component()
{
}

void Destroy(Component* component)
{
    assert(component != nullptr);
    component->enabled = false; // 無効化（ここはUniyと挙動が異なる）
    component->isCalledDestroy = true; // フレームの終わりに削除される
}

}
