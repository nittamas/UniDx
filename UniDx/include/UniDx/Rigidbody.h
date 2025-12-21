#pragma once

#include "Component.h"
#include "Transform.h"
#include "Time.h"
#include "Physics.h"
#include "Bounds.h"
#include "Debug.h"


namespace UniDx {


// --------------------
// Rigidbodyクラス
// --------------------
class Rigidbody : public Component
{
public:
    // 位置。値を直接設定するとテレポートする。
    Property<Vector3> position;

    // 向き
    Property<Quaternion> rotation;

    // 速度
    Vector3 linearVelocity{ 0, 0, 0 };

    // 重力スケール（1.0fで標準重力、0で無重力、負値で逆重力）
    float gravityScale = 1.0f;

    // 質量（0以下は1.0fとして扱う）
    float mass = 1.0f;

    bool isKinematic = false;

    Rigidbody() :
        position(
            [this]() { return position_; },
            [this](Vector3 v) { position_ = v; move_ = Vector3::zero; hasMovePos_ = true; }
        ),
        rotation(
            [this]() { return rotation_; },
            [this](Quaternion q) { rotation_ = q; hasMoveRot_ = true; }
        )
    {
    }

    // 初期化
    virtual void Awake() override
    {
        position_ = transform->position;
        rotation_ = transform->rotation;
    }

    virtual void OnEnable() override
    {
        Physics::getInstance()->registerRigidbody(this);
    }

    virtual void OnDisable() override
    {
        Physics::getInstance()->unregisterRigidbody(this);
    }

    // 指定位置に移動。補間が有効な場合は間の衝突判定を行う。
    void MovePosition(Vector3 pos)
    {
        move_ = pos - position_;
        hasMovePos_ = true;
    }

    // 姿勢を指定。補間が有効な場合は間の衝突判定を行う。
    void MoveRotation(const Quaternion& rot)
    {
        // TODO:補間は未実装
        rotation_ = rot;
        hasMoveRot_ = true;
    }

    // ステップ時間を指定して移動ベクトルを取得
    Vector3 getMoveVector(float step) { return move_ * (Time::fixedDeltaTime > 0 ? step / Time::fixedDeltaTime : 1); }

    // 衝突前の物理更新
    // ここで移動量などを設定しておくが、位置や速度の更新はコリジョン処理の後
    virtual void physicsUpdate()
    {
        if (!enabled) return;

        // 重力適用
        if (gravityScale != 0.0f)
        {
            linearVelocity.y += Physics::gravity * gravityScale * Time::fixedDeltaTime;
        }

        // 位置の直接指定がなければ、移動ベクトルに速度を入れる
        if (!hasMovePos_)
        {
            move_ = linearVelocity * Time::fixedDeltaTime;
        }
    }

    // 移動ベクトルを位置に適用
    virtual void applyMove(float step)
    {
        position_ += getMoveVector(step);

        move_ = Vector3::zero;
        hasMovePos_ = false;
        hasMoveRot_ = false;
    }

    // 位置と速度の補正を適用してTransformに反映
    virtual void solveCorrection(Bounds correctPosition, Bounds correctVelocity)
    {
        // 位置と速度の補正
        position_ += correctPosition.min();
        position_ += correctPosition.max();

        linearVelocity += correctVelocity.min();
        linearVelocity += correctVelocity.max();

        // Transformに位置と姿勢を反映
        transform->position = position_;
        transform->rotation = rotation_;
    }

private:
    Vector3 position_;
    Quaternion rotation_;
    Vector3 move_{ 0, 0, 0 };

    bool hasMovePos_ = false;
    bool hasMoveRot_ = false;
};


} // namespace UniDx
