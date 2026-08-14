#include "pch.h"
#include <UniDx/Component.h>

namespace UniDx{

// コンストラクタ
Component::Component() :
    Object([this]() { return gameObject != nullptr ? gameObject->name : StringId(); }),
    enabled(
        // get
        [this]() { return enabled_ && didAwake_; },

        // set
        [this](bool value) { setEnabled(value); }
    ),
    transform(
        [this]() { return gameObject->transform; }
    ),
    didAwake_(false),
    didStart_(false),
    isCalledDestroy(false),
    enabled_(true),
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
    enabled_ = source.enabled_;
    copyConstruct_ = source.copyConstruct_;
}


void Component::setEnabled(bool value)
{
    if (!enabled_ && value && !isCalledDestroy)
    {
        enabled_ = true;
        // Awakeはアクティブシーンへの接続時に呼ぶ。
        // すでにAwake済みなら、再有効化としてOnEnableを呼ぶ。
        if (didAwake_) { OnEnable(); }
    }
    else if (enabled_ && !value)
    {
        enabled_ = false;
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
    if (enabled_)
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
