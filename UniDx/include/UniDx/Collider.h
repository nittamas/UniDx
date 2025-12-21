#pragma once

#include "Component.h"
#include "Bounds.h"
#include "Physics.h"

namespace UniDx
{

    class Rigidbody;
    class SphereCollider;
    class AABBCollider;

    // --------------------
    // Collider基底クラス
    // --------------------
    class Collider : public Component
    {
    public:
        Rigidbody* attachedRigidbody = nullptr;
        bool isTrigger = false;

        // 物理マテリアル
        float bounciness = 0.75f;

        virtual void OnEnable() override
        {
            attachedRigidbody = findNearestRigidbody(transform);
            Physics::getInstance()->register3d(this);
        }

        virtual void OnDisable() override
        {
            Physics::getInstance()->unregister3d(this);
        }

        // ワールド空間における空間境界を取得
        virtual Bounds getBounds() const = 0;

        // レイキャストチェック
        // 始点が内部のときは false を返す
        virtual bool Raycast(Vector3 origin, Vector3 direction, float maxDistance, RaycastHit* hitInfo = nullptr) = 0;

        // トリガーチェック
        virtual bool intersects(Collider* other) = 0;
        virtual bool intersects(SphereCollider* other) = 0;
        virtual bool intersects(AABBCollider* other) = 0;

        // 衝突チェック
        // 衝突していれば attachedRigidbody に addCorrectPosition(), addCorrectVelocity() で補正する
        virtual bool checkIntersect(Collider* other, PhysicsActor* myActor, PhysicsActor* otherActor) = 0;
        virtual bool checkIntersect(SphereCollider* other, PhysicsActor* myActor, PhysicsActor* otherActor) = 0;
        virtual bool checkIntersect(AABBCollider* other, PhysicsActor* myActor, PhysicsActor* otherActor) = 0;

    private:
        Rigidbody* findNearestRigidbody(Transform* t) const;
    };


    // --------------------
    // AABBCollider
    // --------------------
    class AABBCollider : public Collider
    {
    public:
        Vector3 center;
        Vector3 size;

        AABBCollider(Vector3 c = Vector3::zero) : center(c), size(Vector3(0.5f, 0.5f, 0.5f)) {}

        // ワールド空間における空間境界を取得
        virtual Bounds getBounds() const override;

        // レイキャストチェック
        // 始点が内部のときは false を返す
        virtual bool Raycast(Vector3 origin, Vector3 direction, float maxDistance, RaycastHit* hitInfo = nullptr);

        // トリガーチェック
        virtual bool intersects(Collider* other) { return other->intersects(this); };
        virtual bool intersects(SphereCollider* other);
        virtual bool intersects(AABBCollider* other);

        // 衝突チェック
        // 衝突していれば attachedRigidbody に addCorrectPosition(), addCorrectVelocity() で補正する
        virtual bool checkIntersect(Collider* other, PhysicsActor* myActor, PhysicsActor* otherActor) { return other->checkIntersect(this, otherActor, myActor); }
        virtual bool checkIntersect(SphereCollider* other, PhysicsActor* myActor, PhysicsActor* otherActor);
        virtual bool checkIntersect(AABBCollider* other, PhysicsActor* myActor, PhysicsActor* otherActor);
    };


    // --------------------
    // SphereCollider
    // --------------------
    class SphereCollider : public Collider
    {
    public:
        Vector3 center;
        float radius;

        SphereCollider(Vector3 c = Vector3::zero, float r = 0.5) : center(c), radius(r) {}

        // ワールド空間における空間境界を取得
        virtual Bounds getBounds() const override;

        // レイキャストチェック
        // 始点が内部のときは false を返す
        virtual bool Raycast(Vector3 origin, Vector3 direction, float maxDistance, RaycastHit* hitInfo = nullptr);

        // トリガーチェック
        virtual bool intersects(Collider* other) { return other->intersects(this); };
        virtual bool intersects(SphereCollider* other);
        virtual bool intersects(AABBCollider* other);

        // 衝突チェック
        // 衝突していれば attachedRigidbody に addCorrectPosition(), addCorrectVelocity() で補正する
        virtual bool checkIntersect(Collider* other, PhysicsActor* myActor, PhysicsActor* otherActor) { return other->checkIntersect(this, otherActor, myActor); }
        virtual bool checkIntersect(SphereCollider* other, PhysicsActor* myActor, PhysicsActor* otherActor);
        virtual bool checkIntersect(AABBCollider* other, PhysicsActor* myActor, PhysicsActor* otherActor);
    };


} // namespace UniDx
