#include "pch.h"

#include <limits>
#include <UniDx/Collider.h>
#include <UniDx/Collision.h>
#include <UniDx/Rigidbody.h>

namespace
{

    using namespace UniDx;
    using namespace std;

    constexpr float infinity = numeric_limits<float>::infinity();


    // トリガーチェック
    bool checkTrigger_(SphereCollider* sphere, AABBCollider* aabb)
    {
        // 球の中心（ワールド座標）
        Vector3 sphereCenter = sphere->transform->TransformPoint(sphere->center);
        float sphereRadius = sphere->radius;

        // AABBのBounds
        Bounds aabbBounds = aabb->getBounds();

        // AABB上で球中心に最も近い点
        Vector3 closest = aabbBounds.ClosestPoint(sphereCenter);

        // 最近点と球中心のベクトル
        Vector3 normal = sphereCenter - closest;
        float distSqr = normal.sqrMagnitude();

        // 衝突していない
        return distSqr <= sphereRadius * sphereRadius;
    }

    // 衝突していれば attachedRigidbody に addCorrectPosition(), addCorrectVelocity() で補正する
    bool checkIntersect_(SphereCollider* sphere, AABBCollider* aabb, PhysicsActor* sphereActor, PhysicsActor* aabbActor)
    {
        // 球の中心（ワールド座標）
        Vector3 sphereCenter = sphere->transform->TransformPoint(sphere->center);
        float sphereRadius = sphere->radius;

        // AABBのBounds
        Bounds aabbBounds = aabb->getBounds();

        // AABB上で球中心に最も近い点
        Vector3 closest = aabbBounds.ClosestPoint(sphereCenter);

        // 最近点と球中心のベクトル
        Vector3 normal = sphereCenter - closest;
        float distSqr = normal.sqrMagnitude();

        // 衝突していない
        if (distSqr > sphereRadius * sphereRadius)
            return false;

        // Rigidbody取得
        Rigidbody* rbA = sphere->attachedRigidbody;
        Rigidbody* rbB = aabb->attachedRigidbody;

        // 相対速度
        Vector3 velA = rbA ? rbA->linearVelocity : Vector3::zero;
        Vector3 velB = rbB ? rbB->linearVelocity : Vector3::zero;
        Vector3 relVel = velA - velB;

        // 相対速度が法線方向（離れようとしている）場合は無視
        if (Dot(relVel, normal) > 0)
            return false;

        float dist = std::sqrt(distSqr);
        // 法線（dist==0のときは適当な軸にする）
        Vector3 contactNormal = (dist > 1e-6f) ? (normal / dist) : Vector3(1, 0, 0);

        // penetration（めり込み量）
        float penetration = sphereRadius - dist;

        // 質量取得（0以下は1.0f扱い）
        float massA = (rbA && !rbA->isKinematic) ? (rbA->mass > 0.0f ? rbA->mass : 1.0f) : infinity;
        float massB = (rbB && !rbB->isKinematic) ? (rbB->mass > 0.0f ? rbB->mass : 1.0f) : infinity;
        float totalMass = massA + massB;

        float massAPerTotal = massA != infinity ? massA / totalMass : 1;
        float massBPerTotal = massB != infinity ? massB / totalMass : 1;

        // 補正ベクトル
        Vector3 correctionA = contactNormal * (penetration * massBPerTotal);
        Vector3 correctionB = -contactNormal * (penetration * massAPerTotal);

        // 位置補正
        if (rbA && !rbA->isKinematic && massA != infinity) sphereActor->addCorrectPosition(correctionA);
        if (rbB && !rbB->isKinematic && massB != infinity) aabbActor->addCorrectPosition(correctionB);

        // 跳ね返り係数
        float bounce = sphere->bounciness * aabb->bounciness;

        // 法線方向の速度成分
        float relVelN = Dot(relVel, contactNormal);

        // 反射させる
        Vector3 impulse = -(1.0f + bounce) * relVelN * contactNormal;

        if (rbA && !rbA->isKinematic && massA != infinity) sphereActor->addCorrectVelocity(impulse * massBPerTotal);
        if (rbB && !rbB->isKinematic && massB != infinity) aabbActor->addCorrectVelocity(-impulse * massAPerTotal);

        return true;
    }

}


namespace UniDx
{


    // TransformをたどってRigidbodyを探す
    Rigidbody* Collider::findNearestRigidbody(Transform* t) const
    {
        // 同じGameObjectにRigidbodyがあればそれを登録
        Rigidbody* rb = gameObject->GetComponent<Rigidbody>();
        if (rb != nullptr)
        {
            return rb;
        }

        // なければ親をたどって再帰呼び出し
        if (t->parent != nullptr)
        {
            return findNearestRigidbody(t->parent);
        }

        // ない。このときは動かないCollilderになる
        return nullptr;
    }


    // ワールド空間における空間境界を取得
    Bounds SphereCollider::getBounds() const
    {
        return Bounds(transform->position + transform->TransformVector(center), Vector3(radius, radius, radius));
    }


    // ワールド空間における空間境界を取得
    Bounds AABBCollider::getBounds() const
    {
        return Bounds(transform->position + transform->TransformVector(center), transform->TransformVector(size));
    }


    // トリガーチェック
    bool AABBCollider::intersects(AABBCollider* other)
    {
        return getBounds().Intersects(other->getBounds());
    }


    // トリガーチェック
    bool AABBCollider::intersects(SphereCollider* other)
    {
        return checkTrigger_(other, this);
    }


    // 衝突チェック
    // 衝突していれば attachedRigidbody に addCorrectPosition(), addCorrectVelocity() で補正する
    bool AABBCollider::checkIntersect(AABBCollider* other, PhysicsActor* myActor, PhysicsActor* otherActor)
    {
        return false;
    }


    // 衝突チェック
    // 衝突していれば attachedRigidbody に addCorrectPosition(), addCorrectVelocity() で補正する
    bool AABBCollider::checkIntersect(SphereCollider* other, PhysicsActor* myActor, PhysicsActor* otherActor)
    {
        return checkIntersect_(other, this, otherActor, myActor);
    }


    //
    // Raycast 実装（AABB）
    // - 始点がコライダー内部なら無視する
    //
    bool AABBCollider::Raycast(Vector3 origin, Vector3 direction, float maxDistance, RaycastHit* hitInfo)
    {
        const float eps = 1e-6f;
        Bounds b = getBounds();

        // origin が内部にある場合は Unity と同様に無視する
        if (b.SqrDistance(origin) <= eps * eps)
        {
            return false;
        }

        Vector3 bmin = b.min();
        Vector3 bmax = b.max();

        float tmin = 0.0f;
        float tmax = maxDistance;

        // X axis
        if (fabs(direction.x) < eps)
        {
            if (origin.x < bmin.x || origin.x > bmax.x) return false;
        }
        else
        {
            float inv = 1.0f / direction.x;
            float t1 = (bmin.x - origin.x) * inv;
            float t2 = (bmax.x - origin.x) * inv;
            float tn = std::min(t1, t2);
            float tf = std::max(t1, t2);
            tmin = std::max(tmin, tn);
            tmax = std::min(tmax, tf);
            if (tmin > tmax) return false;
        }

        // Y axis
        if (fabs(direction.y) < eps)
        {
            if (origin.y < bmin.y || origin.y > bmax.y) return false;
        }
        else
        {
            float inv = 1.0f / direction.y;
            float t1 = (bmin.y - origin.y) * inv;
            float t2 = (bmax.y - origin.y) * inv;
            float tn = std::min(t1, t2);
            float tf = std::max(t1, t2);
            tmin = std::max(tmin, tn);
            tmax = std::min(tmax, tf);
            if (tmin > tmax) return false;
        }

        // Z axis
        if (fabs(direction.z) < eps)
        {
            if (origin.z < bmin.z || origin.z > bmax.z) return false;
        }
        else
        {
            float inv = 1.0f / direction.z;
            float t1 = (bmin.z - origin.z) * inv;
            float t2 = (bmax.z - origin.z) * inv;
            float tn = std::min(t1, t2);
            float tf = std::max(t1, t2);
            tmin = std::max(tmin, tn);
            tmax = std::min(tmax, tf);
            if (tmin > tmax) return false;
        }

        float tHit = tmin;
        if (tHit < 0.0f) tHit = 0.0f;

        if (tHit <= maxDistance)
        {
            if (hitInfo)
            {
                Vector3 hitPoint = origin + direction * tHit;

                Vector3 normal = Vector3::zero;
                const float normEps = 1e-3f;
                if (fabs(hitPoint.x - bmin.x) < normEps) normal = Vector3(-1, 0, 0);
                else if (fabs(hitPoint.x - bmax.x) < normEps) normal = Vector3(1, 0, 0);
                else if (fabs(hitPoint.y - bmin.y) < normEps) normal = Vector3(0, -1, 0);
                else if (fabs(hitPoint.y - bmax.y) < normEps) normal = Vector3(0, 1, 0);
                else if (fabs(hitPoint.z - bmin.z) < normEps) normal = Vector3(0, 0, -1);
                else if (fabs(hitPoint.z - bmax.z) < normEps) normal = Vector3(0, 0, 1);
                else
                {
                    Vector3 invDir = -direction;
                    float len = std::sqrt(invDir.x * invDir.x + invDir.y * invDir.y + invDir.z * invDir.z);
                    if (len > eps) normal = invDir / len;
                }

                hitInfo->collider = this;
                hitInfo->point = hitPoint;
                hitInfo->normal = normal;
                hitInfo->distance = tHit;
            }
            return true;
        }

        return false;
    }


    // トリガーチェック
    bool SphereCollider::intersects(AABBCollider* other)
    {
        return checkTrigger_(this, other);
    }


    // トリガーチェック
    bool SphereCollider::intersects(SphereCollider* other)
    {
        Vector3 centerA = transform->TransformPoint(center);
        Vector3 centerB = other->transform->TransformPoint(other->center);
        float radiusAB = radius + other->radius;

        // 中心距離が半径の合計より離れていれば当たっていない
        return SqrDistance(centerA, centerB) <= radiusAB * radiusAB;
    }


    // 衝突チェック
    // 衝突していれば attachedRigidbody に addCorrectPosition(), addCorrectVelocity() で補正する
    bool SphereCollider::checkIntersect(AABBCollider* other, PhysicsActor* myActor, PhysicsActor* otherActor)
    {
        return checkIntersect_(this, other, myActor, otherActor);
    }


    // 衝突チェック
    // 衝突していれば attachedRigidbody に addCorrectPosition(), addCorrectVelocity() で補正する
    bool SphereCollider::checkIntersect(SphereCollider* other, PhysicsActor* myActor, PhysicsActor* otherShap)
    {
        Vector3 centerA = transform->TransformPoint(center);
        float radiusA = radius;
        Vector3 centerB = other->transform->TransformPoint(other->center);
        float radiusB = other->radius;

        // 中心距離が半径の合計より離れていれば当たっていない
        if (Distance(centerA, centerB) > radiusA + radiusB)
            return false;

        // めり込みの深さ
        float penetration = radiusA + radiusB - Distance(centerA, centerB);

        // 中心の差
        Vector3 sub = centerB - centerA;

        // それぞれの位置補正
        Vector3 addB = sub.normalized();
        addB *= penetration * 0.5f;

        otherShap->addCorrectPosition(addB);

        Vector3 addA = (-sub).normalized();
        addA *= penetration * 0.5f;

        myActor->addCorrectPosition(addA);

        // 跳ね返り計算
        Vector3 va = attachedRigidbody->linearVelocity;
        Vector3 vb = other->attachedRigidbody->linearVelocity;

        // 相対速度
        Vector3 relV = va - vb;

        Vector3 normal = sub.normalized();
        if (Dot(relV, normal) < 0)
        {
            return false;
        }

        // 跳ね返り係数
        float bounce = bounciness * other->bounciness;

        Vector3 relVNormal = normal * Dot(relV, normal);
        myActor->addCorrectVelocity(relVNormal * -bounce);
        otherShap->addCorrectVelocity(relVNormal * bounce);

        return true;
    }


    //
    // Raycast 実装（Sphere）
    // - 始点がコライダー内部なら無視する
    //
    bool SphereCollider::Raycast(Vector3 origin, Vector3 direction, float maxDistance, RaycastHit* hitInfo)
    {
        const float eps = 1e-6f;
        Vector3 centerWorld = transform->TransformPoint(center);

        // origin が内部にある場合は無視
        float distSqr = SqrDistance(origin, centerWorld);
        if (distSqr <= radius * radius)
        {
            return false;
        }

        // 二次方程式: (d·d)t^2 + 2(d·oc)t + (oc·oc - r^2) = 0
        Vector3 oc = origin - centerWorld;
        float a = Dot(direction, direction);
        float b = 2.0f * Dot(direction, oc);
        float c = Dot(oc, oc) - radius * radius;

        float disc = b * b - 4.0f * a * c;
        if (disc < 0.0f) return false;

        float sqrtD = std::sqrt(disc);
        float t0 = (-b - sqrtD) / (2.0f * a);
        float t1 = (-b + sqrtD) / (2.0f * a);

        float t = std::numeric_limits<float>::infinity();
        if (t0 >= 0.0f) t = t0;
        else if (t1 >= 0.0f) t = t1; // origin 内部なら除外済みなので通常はこちらは有効になることは少ない

        if (!(t >= 0.0f) || t > maxDistance) return false;

        if (hitInfo)
        {
            Vector3 hitPoint = origin + direction * t;
            Vector3 normal = hitPoint - centerWorld;
            float len = normal.magnitude();
            if (len > eps) normal /= len;
            else normal = Vector3(1, 0, 0);

            hitInfo->collider = this;
            hitInfo->point = hitPoint;
            hitInfo->normal = normal;
            hitInfo->distance = t;
        }

        return true;
    }



}
