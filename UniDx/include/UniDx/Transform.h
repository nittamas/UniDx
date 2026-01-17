/**
 * @file Transform.h
 * @brief GameObjectの位置、回転、スケールを扱い、階層構造を実現する
 */
#pragma once

#include <memory>

#include "UniDxDefine.h"
#include "Component.h"
#include "GameObject.h"


namespace UniDx {


/**
 * @brief GameObjectの位置、回転、スケールを扱い、階層構造を実現するクラス
 * GameObjectに必ず１つアタッチされている
 */
class Transform : public Component
{
public:
    typedef std::vector<unique_ptr<GameObject>> GameObjectContainer;

    // ローカルの姿勢
    Property<Vector3> localPosition;
    Property<Quaternion> localRotation;
    Property<Vector3> localScale;

    // ワールド空間のプロパティ
    Property<Vector3> position;
    Property<Quaternion> rotation;
    Property<Vector3> forward;
    Property<Vector3> up;
    Property<Vector3> right;

    Transform* parent = nullptr;

    const GameObjectContainer& getChildGameObjects() const { return children; }

    /// @brief 親の変更（すでに親を設定している場合）
    GameObject* SetParent(Transform* newParent);

    /// @brief 親のいないTransformを持つGameObjectに親を設定
    static void SetParent(unique_ptr<GameObject> gameObjectPtr, Transform* newParent);

    /// @brief 子の数を取得
    size_t childCount() const { return children.size(); }

    /// @brief 子を取得
    Transform* GetChild(size_t index) const;

    /// @brief ローカル座標系から親座標系への変換行列
    const Matrix4x4& localMatrix() const;

    /// @brief ワールド座標系への変換行列
    const Matrix4x4& localToWorldMatrix() const {
        updateMatrices();
        return m_worldMatrix;
    }

    Transform();

    virtual ~Transform();

    /// @brief ローカル空間の方向ベクトルをワールド空間の方向ベクトルに変換
    Vector3 TransformDirection(Vector3 localDirection) const;

    /// @brief ローカル空間のベクトルをワールド空間のベクトルに変換（スケール・回転のみ、平行移動なし）
    Vector3 TransformVector(Vector3 localVector) const {
        // TransformDirectionと同じ実装（Unityと同様の意味）
        return TransformDirection(localVector);
    }

    /// @brief ローカル空間の座標をワールド空間の座標に変換
    Vector3 TransformPoint(Vector3 localPoint) const {
        return localPoint * localToWorldMatrix();
    }

private:
    // ダーティフラグと行列
    mutable bool m_dirty = true;
    mutable Matrix4x4 m_localMatrix = Matrix4x4::identity;
    mutable Matrix4x4 m_worldMatrix = Matrix4x4::identity;

    bool dirtyInHierarchy() const { return m_dirty || parent && parent->dirtyInHierarchy(); }

    Vector3 _localPosition{ 0,0,0 };
    Quaternion _localRotation = Quaternion::identity;
    Vector3 _localScale{ 1,1,1 };

    // 子GameObject
    // トップ以外のGameObjectはTransformによって保持される
    GameObjectContainer children;

    // 行列の更新
    void updateMatrices() const;
};

} // namespace UniDx
