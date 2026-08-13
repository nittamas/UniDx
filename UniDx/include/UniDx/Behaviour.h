/**
 * @file Behaviour.h
 * @brief GameObjectの挙動を記述する基底コンポーネント
 */
#pragma once

#include "Component.h"
#include "Transform.h"

namespace UniDx {

class Collider;
struct Collision;

/// @brief GameObjectの挙動を記述する基底コンポーネント。UnityのMonoBehaviour相当
class Behaviour : public Component
{
public:
    virtual void FixedUpdate() {}
    virtual void Update() {}
    virtual void LateUpdate() {}
    virtual void OnTriggerEnter(Collider* other) {}
    virtual void OnTriggerStay(Collider* other) {}
    virtual void OnTriggerExit(Collider* other) {}
    virtual void OnCollisionEnter(const Collision& collision) {}
    virtual void OnCollisionStay(const Collision& collision) {}
    virtual void OnCollisionExit(const Collision& collision) {}

    virtual ~Behaviour() = default;
};


} // namespace UniDx
