#pragma once

#include <UniDx.h>
#include <UniDx/Rigidbody.h>

using namespace UniDx;

class Player : public UniDx::Behaviour
{
public:
    virtual void OnEnable() override;
    virtual void Update() override;
    virtual void OnTriggerEnter(Collider* other) override;
    virtual void OnTriggerStay(Collider* other) override;
    virtual void OnTriggerExit(Collider* other) override;
    virtual void OnCollisionEnter(const Collision& collision) override;
    virtual void OnCollisionStay(const Collision& collision) override;
    virtual void OnCollisionExit(const Collision& collision) override;

    UniDx::Rigidbody* rb = nullptr;

private:
    std::vector<UniDx::Transform*> bones;
    std::vector<UniDx::Quaternion> initialRotate;
    float animFrame;
};
