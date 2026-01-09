#pragma once

#include "Player.h"

#include <UniDx/Input.h>
#include <UniDx/Collider.h>
#include <UniDx/Time.h>

#include "MainGame.h"

using namespace DirectX;
using namespace UniDx;

namespace
{
    void debugNode(GameObject* p, std::string head)
    {
        std::string str = head;
        str.append(p->name.get().c_str());
        Debug::Log(str);
        str = head;
        str.append( std::to_string(p->transform->position.get().y));
        Debug::Log(str);
        str = head + " ";
        for (int i = 0; i < p->transform->childCount(); ++i)
        {
            debugNode(p->transform->GetChild(i)->gameObject, str);
        }
    }

    const char* PartsName[] =
    {
        "LeftShoulder",
        "RightShoulder",
        "LeftUpperLeg",
        "RightUpperLeg"
    };
}


void Player::OnEnable()
{
    rb = GetComponent<Rigidbody>(true);
    assert(rb != nullptr);

    rb->gravityScale = 1.5f;
    GetComponent<Collider>(true)->bounciness = 0.0f;

    for (int i = 0; i < (int)Parts::Max; ++i)
    {
        GameObject * o = gameObject->Find([i](GameObject* p) { return p->name.get().c_str() == PartsName[i]; });
        if (o != nullptr)
        {
            parts[i] = o->transform;
        }
    }
    animFrame = 0.0f;
}


void Player::Update()
{
    const float moveSpeed = 4;

    // 操作方向
    Vector3 cont;
    if (Input::GetKey(Keyboard::A))
    {
        cont.x = -1.0f;
    }
    else if (Input::GetKey(Keyboard::D))
    {
        cont.x = 1.0f;
    }
    else
    {
        cont.x = 0.0f;
    }

    if (Input::GetKey(Keyboard::S))
    {
        cont.z = -1.0f;
    }
    else if (Input::GetKey(Keyboard::W))
    {
        cont.z = 1.0f;
    }
    else
    {
        cont.z = 0.0f;
    }
    cont = cont.normalized();

    // カメラ方向を考慮して速度ベクトルを計算
    Vector3 camF = Camera::main->transform->forward;
    float camAngle = std::atan2(camF.x, camF.z) * UniDx::Rad2Deg;
    Vector3 velocity = (cont.normalized() * moveSpeed) * Quaternion::AngleAxis(camAngle, Vector3::up);
    float vAngle = std::atan2(velocity.x, velocity.z) * UniDx::Rad2Deg;

    rb->linearVelocity = velocity;
    if (cont != Vector3::zero)
    {
        rb->rotation = Quaternion::Euler(0, vAngle, 0);
    }

    // アニメ（未対応）
    animFrame += cont.magnitude();
}


void Player::OnTriggerEnter(Collider* other)
{
}


void Player::OnTriggerStay(Collider* other)
{
}


void Player::OnTriggerExit(Collider* other)
{
}


void Player::OnCollisionEnter(const Collision& collision)
{
    if (collision.collider->name.get() == StringId::intern("Coin"))
    {
        MainGame::getInstance()->AddScore(1);
        Destroy(collision.collider->gameObject);
    }
}


void Player::OnCollisionStay(const Collision& collision)
{
}

void Player::OnCollisionExit(const Collision& collision)
{
}

