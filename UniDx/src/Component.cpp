#include "pch.h"
#include <UniDx/Component.h>

namespace UniDx{

// コンストラクタ
Component::Component() :
    Object([this]() { return gameObject != nullptr ? gameObject->name : StringId(); }),
    enabled(
        // get
        [this]() { return _enabled && didAwake_; },

        // set
        [this](bool value) { setEnabled(value); }
    ),
    transform(
        [this]() { return gameObject->transform; }
    ),
    didAwake_(false),
    didStart_(false),
    isCalledDestroy(false),
    _enabled(true),
    copyConstruct_(nullptr)
{
}


// コピーコンストラクタ
// Propertyのラムダはコピーせず、複製先のthisへ張り直す
Component::Component(const Component& source) : Component()
{
    copyComponentStateFrom(source);
}


void Component::copyComponentStateFrom(
    const Component& source)
{
    _enabled = source._enabled;
    copyConstruct_ = source.copyConstruct_;
}


void Component::setEnabled(bool value)
{
    if (!_enabled && value && !isCalledDestroy)
    {
        _enabled = true;
        // Awakeはアクティブシーンへの接続時に呼ぶ。
        // すでにAwake済みなら、再有効化としてOnEnableを呼ぶ。
        if (didAwake_) { OnEnable(); }
    }
    else if (_enabled && !value)
    {
        _enabled = false;
        if (didAwake_) { OnDisable(); }
    }
}

std::unique_ptr<Component> Component::copyConstruct() const
{
    if (copyConstruct_ == nullptr) return nullptr;
    return copyConstruct_(*this);
}

void Component::doDestroy()
{
    isCalledDestroy = true; // 以降で enabled=true は無効
    if (_enabled)
    {
        enabled = false; // 無効化（この中でOnDisable()が呼ばれる）
    }
    if (didAwake_)
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
    component->isCalledDestroy = true; // フレームの終わりに削除される
}

}
