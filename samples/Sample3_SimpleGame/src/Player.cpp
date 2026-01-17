#pragma once

#include "Player.h"

#include <UniDx/Input.h>
#include <UniDx/Collider.h>
#include <UniDx/Time.h>
#include <UniDx/PrimitiveRenderer.h>

#include "MainGame.h"

using namespace DirectX;
using namespace UniDx;

namespace
{
    const StringId CoinName = StringId::intern("Coin");

    // アニメショーンさせるボーン名
    const StringId BoneName[] =
    {
        StringId::intern("LeftUpperArm"),
        StringId::intern("RightUpperArm"),
        StringId::intern("LeftUpperLeg"),
        StringId::intern("RightUpperLeg"),
        StringId::intern("Tail")
    };
    // アニメーションさせる角度の範囲（pitch, yaw, roll）
    const Vector3 Range[] =
    {
        Vector3( 80,  0,  0),
        Vector3(-80,  0,  0),
        Vector3( 30,  0, 45),
        Vector3(-30,  0, 45),
        Vector3(  0,-60,  0),
    };
    // アニメーションさせる角度のオフセット（pitch, yaw, roll）
    const Vector3 Offset[] =
    {
        Vector3(  0,  0, 30),
        Vector3(  0,  0,-30),
        Vector3(  0,  5,  0),
        Vector3(  0, -5,  0),
        Vector3( 20,  0,  0),
    };
    constexpr size_t BoneMax = sizeof(BoneName) / sizeof(StringId);
    constexpr float animSpeed = 0.07f;
}


void Player::OnEnable()
{
    rb = GetComponent<Rigidbody>(true);
    assert(rb != nullptr);

    rb->gravityScale = 1.5f;
    GetComponent<Collider>(true)->bounciness = 0.0f;

    // アニメーションさせるボーンを検索し、初期姿勢の回転を記録
    bones.resize(BoneMax);
    initialRotate.resize(BoneMax);
    for (int i = 0; i < BoneMax; ++i)
    {
        GameObject * o = gameObject->Find([i](GameObject* p) { return p->name == BoneName[i]; });
        if (o != nullptr)
        {
            bones[i] = o->transform;
            initialRotate[i] = o->transform->localRotation;
        }
    }
    animFrame = 0.0f;
}


void Player::Update()
{
    const float moveSpeed = 5;

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
    Vector3 velocity = cont * moveSpeed * Quaternion::AngleAxis(camAngle, Vector3::up);
    float vAngle = std::atan2(velocity.x, velocity.z) * UniDx::Rad2Deg;

    rb->linearVelocity = velocity;
    if (cont != Vector3::zero)
    {
        rb->rotation = Quaternion::Euler(0, vAngle, 0);
    }

    // プログラムアニメ
    animFrame += cont.magnitude();
    for(int i = 0; i < bones.size(); ++i)
    {
        auto& bone = bones[i];
        if(bone == nullptr) continue;

        Quaternion r = bone->localRotation;
        float sn = std::sin(animFrame * animSpeed);
        r = Quaternion::Euler(
            sn * Range[i].x + Offset[i].x,
            sn * Range[i].y + Offset[i].y,
            sn * Range[i].z + Offset[i].z);
        bone->localRotation = r * initialRotate[i]; // 頂点×回転×初期姿勢
    }
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


// コライダーに当たったときのコールバック
void Player::OnCollisionEnter(const Collision& collision)
{
    if (collision.collider->name == CoinName)
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

