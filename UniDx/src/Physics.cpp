#include "pch.h"
#include <UniDx/Physics.h>

#include <numbers>
#include <algorithm>

#include <UniDx/Collider.h>
#include <UniDx/Rigidbody.h>
#include <PhysicsGrid.h>

#define UNIDX_PHYSICS_USE_GRID true

namespace UniDx
{

    using namespace std;

    // 初期化
    void PhysicsShape::initialize(Collider* collider)
    {
        collider_ = collider;
        // moveBounds
    }

    // 衝突対象の新旧を調べて OnTrigger～, OnCollidion～ を呼ぶ
    void PhysicsShape::collideCallback()
    {
        // トリガーコールバック
        for (auto other : triggersNew_)
        {
            auto inOld = std::ranges::find(triggers_, other);
            if (inOld == triggers_.end())
            {
                // 以前のリストに含まれていない＝新規
                getCollider()->gameObject->onTriggerEnter(other);
                if (!isValid()) return;
            }
            else
            {
                // 以前のリストに含まれていれば、一旦削除
                triggers_.erase(inOld);
            }

            // 新しいほうに含まれているので、Stay
            getCollider()->gameObject->onTriggerStay(other);
            if (!isValid()) return;
        }

        // 新しいリストになくて古いほうに残っている=離れた
        for (auto other : triggers_)
        {
            getCollider()->gameObject->onTriggerExit(other);
            if (!isValid()) return;
        }

        // 古いほうを削除して新しいほうを古いほうに
        triggers_.clear();
        std::swap(triggers_, triggersNew_);

        // 衝突コールバック
        for (const auto& collision : collisionsNew_)
        {
            const auto& inOld = std::ranges::find_if(collisions_, [collision](auto i) {return i.collider == collision.collider; });
            if (inOld == collisions_.end())
            {
                // 以前のリストに含まれていない＝新規
                getCollider()->gameObject->onCollisionEnter(collision);
                if (!isValid()) return;
            }
            else
            {
                // 以前のリストに含まれていれば、一旦削除
                collisions_.erase(inOld);
            }

            // 新しいほうに含まれているので、Stay
            getCollider()->gameObject->onCollisionStay(collision);
            if (!isValid()) return;
        }

        // 新しいリストになくて古いほうに残っている=離れた
        for (auto col : collisions_)
        {
            getCollider()->gameObject->onCollisionExit(col);
            if (!isValid()) return;
        }

        // 古いほうを削除して新しいほうを古いほうに
        collisions_.clear();
        std::swap(collisions_, collisionsNew_);
    }

    // コンストラクタ
    Physics::Physics()
    {
        // 毎フレームクリアされるデータはできるだけ再利用する
        // 最初にある程度の数を予約
        potentialPairs.reserve(128);
        potentialPairsTrigger.reserve(128);
#if UNIDX_PHYSICS_USE_GRID
        physicsGrid = make_unique<PhysicsGrid>(MakeMemberAction(this, &Physics::checkBounds));
#endif

    }

    // Rigidbodyを登録
    void Physics::registerRigidbody(Rigidbody* rigidbody)
    {
        PhysicsActor temp(rigidbody);
        physicsActors.insert(std::make_pair(rigidbody, temp));
    }


    // 3D形状を持ったコライダーの登録を解除
    void Physics::unregisterRigidbody(Rigidbody* rigidbody)
    {
        physicsActors.erase(rigidbody);
    }


    // 3D形状を持ったコライダーを登録
    void Physics::register3d(Collider* collider)
    {
        for (size_t i = 0; i < physicsShapes.size(); ++i)
        {
            if (!physicsShapes[i].isValid())
            {
                physicsShapes[i].initialize(collider);
                return;
            }
            if (physicsShapes[i].getCollider() == collider)
            {
                return; // 登録済み
            }
        }

        // 無効化されたものがなければ追加
        physicsShapes.push_back(PhysicsShape());
        physicsShapes.back().initialize(collider);
    }


    // 3D形状を持ったコライダーの登録を解除
    void Physics::unregister3d(Collider* collider)
    {
        for (size_t i = 0; i < physicsShapes.size(); ++i)
        {
            if (physicsShapes[i].getCollider() == collider)
            {
                physicsShapes[i].setInvalid();
                return;
            }
        }
    }


    // 物理計算準備
    void Physics::initializeSimulate(float step)
    {
        // 無効になっているものをvectorから削除
        for (auto it = physicsActors.begin(); it != physicsActors.end();)
        {
            if (!it->second.isValid())
            {
                it = physicsActors.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // 無効になっているシェイプを削除
        for (vector<PhysicsShape>::iterator it = physicsShapes.begin(); it != physicsShapes.end();)
        {
            if (!it->isValid())
            {
                it = physicsShapes.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // Rigidbodyの更新
        for (auto& act : physicsActors)
        {
            act.second.getRigidbody()->physicsUpdate();
            act.second.initCorrectBounds();
        }

        // Shapeの移動Boundsと次に当たるコライダーを初期化
        for (auto& shape : physicsShapes)
        {
            shape.initOtherNew();

            Bounds bounds = shape.getCollider()->getBounds();
            auto rb = shape.getCollider()->attachedRigidbody;
            if (rb != nullptr)
            {
                bounds.Encapsulate(bounds.min() + rb->getMoveVector(step));
                bounds.Encapsulate(bounds.max() + rb->getMoveVector(step));
            }
            shape.moveBounds = bounds;
            Rigidbody* r = shape.getCollider()->attachedRigidbody;
            if (r != nullptr)
            {
                if (shape.actor == nullptr)
                {
                    shape.actor = &physicsActors.at(r);
                }
            }
            else
            {
                shape.actor = nullptr;
            }
        }
    }


    // 位置補正法（射影法）による物理計算のシミュレート
    void Physics::simulatePositionCorrection(float step)
    {
        auto start = std::chrono::system_clock::now(); // 開始時刻を記録

        initializeSimulate(step);

        static double totalTime = 0.0;
        static double totalInsert = 0.0;
        static int timeCount = 0;

        // まずは当たりそうなペアをAABBで判定して抽出
        potentialPairs.clear();
        potentialPairsTrigger.clear();

#if UNIDX_PHYSICS_USE_GRID
        physicsGrid->update(physicsShapes);
        auto insert = std::chrono::system_clock::now(); // 終了時刻を記録
        totalInsert += std::chrono::duration_cast<std::chrono::microseconds>(insert - start).count() * 0.001;
        physicsGrid->gatherPairs();
#else
        for (size_t i = 0; i < physicsShapes.size(); ++i)
        {
            for (size_t j = i + 1; j < physicsShapes.size(); ++j)
            {
                checkBounds(&physicsShapes[i], &physicsShapes[j]);
            }
        }
#endif
        auto end = std::chrono::system_clock::now(); // 終了時刻を記録
        std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        totalTime += elapsed.count() * 0.001;
        timeCount++;
        if (timeCount % 100 == 0)
        {
//            Debug::Log(totalInsert);
//            Debug::Log(totalTime);
            totalInsert = 0.0;
            totalTime = 0.0;
            timeCount = 0;
        }

        // 先に位置を更新する
        for (auto& act : physicsActors)
        {
            act.second.getRigidbody()->applyMove(step);
        }

        // トリガーチェックする
        for (auto& pair : potentialPairsTrigger)
        {
            if (pair.first->getCollider()->intersects(pair.second->getCollider()))
            {
                pair.first->addTrigger(pair.second->getCollider());
                pair.second->addTrigger(pair.first->getCollider());
            }
        }

        // 衝突をチェックする
        for (auto& pair : potentialPairs)
        {
            if (pair.first->getCollider()->checkIntersect(pair.second->getCollider(), pair.first->actor, pair.second->actor))
            {
                Collision ca;
                ca.collider = pair.second->getCollider();
                pair.first->addCollide(ca);

                Collision cb;
                cb.collider = pair.first->getCollider();
                pair.second->addCollide(cb);
            }
        }

        // 衝突で生じた補正を含めて位置と速度を解決する
        for (auto& act : physicsActors)
        {
            act.second.getRigidbody()->solveCorrection(act.second.getCorrectPositionBounds(), act.second.getCorrectVelocityBounds());
        }

        // OnTrigger～, OnCollision～等のコールバックを呼び出す
        // TODO: 当たったRigidbodyがついているGameObjectでも呼び出す
        for (auto& shape : physicsShapes)
        {
            if (shape.isValid())
            {
                shape.collideCallback();
            }
        }
    }

    void Physics::checkBounds(PhysicsShape* shape1, PhysicsShape* shape2)
    {
        if (shape1->moveBounds.Intersects(shape2->moveBounds))
        {
            auto rbA = shape1->getCollider()->attachedRigidbody;
            auto rbB = shape2->getCollider()->attachedRigidbody;

            // 同じ Rigidbody に属しているコンパウンド同士は自己衝突なのでスキップ
            if (rbA && rbA == rbB) return;

            // ペアを記憶
            if (shape1->getCollider()->isTrigger || shape2->getCollider()->isTrigger)
            {
                // トリガー
                potentialPairsTrigger.push_back({ shape1, shape2 });
            }
            else
            {
                // コリジョン
                potentialPairs.push_back({ shape1, shape2 });
            }
        }
    }

    // 物理計算のシミュレート（未完成）
    void Physics::simulate(float step)
    {
        initializeSimulate(step);

        // まずは当たりそうなペアをAABBで判定して抽出
        potentialPairs.clear();
        for (size_t i = 0; i < physicsShapes.size(); ++i)
        {
            for (size_t j = i + 1; j < physicsShapes.size(); ++j)
            {
                if (physicsShapes[i].moveBounds.Intersects(physicsShapes[j].moveBounds))
                {
                    // 同じ Rigidbody に属しているコンパウンド同士は自己衝突なのでスキップ
                    auto rbA = physicsShapes[i].getCollider()->attachedRigidbody;
                    auto rbB = physicsShapes[j].getCollider()->attachedRigidbody;
                    if (rbA && rbA == rbB) continue;

                    // ペアを記憶。ここでは詳細判定しない
                    potentialPairs.push_back({ &physicsShapes[i], &physicsShapes[j] });
                }
            }
        }

        // 形状ごとに実衝突を確定する
        manifolds.clear();
        for (auto& pair : potentialPairs)
        {
            ContactManifold m;
            //        if (intersectShapes(*pair.first, *pair.second, &m)) {  // ← ここが Narrow-phase
            //            manifolds.push_back(m);
            //        }
        }

        // ソルバ
        for (auto& m : manifolds)
        {
            Rigidbody* rbA = m.a->getCollider()->attachedRigidbody;
            Rigidbody* rbB = m.b->getCollider()->attachedRigidbody;

            // 速度レベルの反発インパルス (Impulses) をかける
            solveVelocityConstraint(rbA, rbB, m);

            // 位置のめり込みを少し戻す (Baumgarte / Position correction)
            solvePositionConstraint(rbA, rbB, m);
        }
    }


    void Physics::solveVelocityConstraint(Rigidbody* A, Rigidbody* B, const ContactManifold& m)
    {
        const float restitution = 0.2f;   // 反発係数

        for (int i = 0; i < m.numContacts; ++i)
        {
        }
    }


    void Physics::solvePositionConstraint(Rigidbody* A, Rigidbody* B, const ContactManifold& m)
    {

    }

    // Raycast
    bool Physics::Raycast(Vector3 origin, Vector3 direction, float maxDistance,
        RaycastHit* hitInfo, std::function<bool(const Collider*)> filter)
    {
        // 無効な方向や負の距離はヒットしない
        const float eps = 1e-6f;
        if (maxDistance <= 0.0f) return false;
        if (fabs(direction.x) < eps && fabs(direction.y) < eps && fabs(direction.z) < eps) return false;

        bool hitAny = false;
        float bestT = std::numeric_limits<float>::infinity();

        // 各 Collider の実装された Raycast を呼ぶ（Collider 側で始点内部は除外される）
        for (const auto& shape : physicsShapes)
        {
            if (!shape.isValid()) continue;
            Collider* col = shape.getCollider();
            if (!col) continue;

            if (filter && !filter(col)) continue; // フィルタで除外

            RaycastHit localHit;
            if (col->Raycast(origin, direction, maxDistance, &localHit))
            {
                // closest を選択（t を比較）
                if (localHit.distance < bestT)
                {
                    bestT = localHit.distance;
                    if (hitInfo != nullptr)
                    {
                        *hitInfo = localHit;
                    }
                    hitAny = true;
                }
            }
        }

        return hitAny;
    }

} // UniDx
