#pragma once

#include <vector>
#include <array>
#include <map>

#include "Property.h"
#include "Singleton.h"
#include "Bounds.h"
#include "Collision.h"

namespace UniDx
{

class Collider;
class Rigidbody;
class PhysicsShape;


struct Contact
{
//    Vector3 point;
    Vector3 normal;     // from A to B
    float   penetration;
};

struct ContactManifold
{
    PhysicsShape* a;
    PhysicsShape* b;
    std::array<Contact, 4> contacts;  // 1〜4点
    int numContacts;
};

class AABBGeometory;
class SpheresGeometory;
class CapsulesGeometory;
class BoxGeometory;
class PhysicsGrid;


// --------------------
// PhysicsActor
// --------------------
class  PhysicsActor
{
public:
    struct Less {
        bool operator()(const PhysicsActor& lhs, const PhysicsActor& rhs) const {
            return lhs.getRigidbody() < rhs.getRigidbody();
        }
    };

    explicit PhysicsActor(Rigidbody* rigidbody) : rigidbody_(rigidbody) {}

    Rigidbody* getRigidbody() const { return rigidbody_; }
    bool isValid() const { return rigidbody_ != nullptr; }
    void setInvalid() { rigidbody_ = nullptr; }

    Bounds getCorrectPositionBounds() const { return correctPositionBounds; }
    Bounds getCorrectVelocityBounds() const { return correctVelocityBounds; }

    // 補正用のBoundsを初期化
    void initCorrectBounds()
    {
        correctPositionBounds.Center = Vector3::zero;
        correctPositionBounds.extents = Vector3::zero;
        correctVelocityBounds.Center = Vector3::zero;
        correctVelocityBounds.extents = Vector3::zero;
    }

    // 位置を補正する差分ベクトルを登録
    void addCorrectPosition(Vector3 vec)
    {
        correctPositionBounds.Encapsulate(vec);
    }

    // 速度を補正する差分ベクトルを登録
    void addCorrectVelocity(Vector3 vec)
    {
        correctVelocityBounds.Encapsulate(vec);
    }

private:
    Rigidbody* rigidbody_;
    Bounds correctPositionBounds;
    Bounds correctVelocityBounds;
};


// --------------------
// PhysicsShape
// --------------------
class  PhysicsShape
{
public:
    void initialize(Collider* collider);

    Bounds moveBounds;  // コライダーの bounds に移動量を広げた範囲
    PhysicsActor* actor;

    Collider* getCollider() const { return collider_; }
    bool isValid() const { return collider_ != nullptr; }
    void setInvalid() { collider_ = nullptr; }
    void initOtherNew() { triggersNew_.clear(); collisionsNew_.clear(); }
    void addCollide(const Collision& col) { collisionsNew_.push_back(col); }
    void addTrigger(Collider* other) { triggersNew_.push_back(other); }
    void collideCallback();

private:
    Collider* collider_;

    std::vector<Collision> collisions_;
    std::vector<Collision> collisionsNew_;
    std::vector<Collider*> triggers_;
    std::vector<Collider*> triggersNew_;
};


// --------------------
// Physics
// --------------------
class Physics : public Singleton<Physics>
{
public:
    typedef std::pair<PhysicsShape*, PhysicsShape*> PotentialPair;

    static inline float gravity = -9.81f;

    Physics();

    void simulate(float setp);
    void simulatePositionCorrection(float step);

    void registerRigidbody(Rigidbody* rigidbody);
    void unregisterRigidbody(Rigidbody* rigidbody);
    void register3d(Collider* collider);
    void unregister3d(Collider* collider);

    /**
     * @brief origin, direction, maxDistance, filter (デフォルト nullptr => 全て含める)
     * @return コライダーにヒットしたとき true
     */
    bool Raycast(Vector3 origin, Vector3 direction, float maxDistance,
        RaycastHit* hitInfo = nullptr, std::function<bool(const Collider*)> filter = nullptr);

    void checkBounds(PhysicsShape* shape1, PhysicsShape* shape2);

private:
    std::vector<PotentialPair> potentialPairs;
    std::vector<PotentialPair> potentialPairsTrigger;

    std::vector<ContactManifold> manifolds;

    std::map<Rigidbody*, PhysicsActor> physicsActors;
    std::vector<PhysicsShape> physicsShapes;
    std::unique_ptr<PhysicsGrid> physicsGrid;

    void initializeSimulate(float step);
    void solveVelocityConstraint(Rigidbody* A, Rigidbody* B, const ContactManifold& m);
    void solvePositionConstraint(Rigidbody* A, Rigidbody* B, const ContactManifold& m);
};

}
