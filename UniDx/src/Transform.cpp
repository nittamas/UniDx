#include "pch.h"

#include <SimpleMath.h>

namespace UniDx
{

// コンストラクタ
Transform::Transform()
    : localPosition(
        [this]() { return _localPosition; },
        [this](Vector3 v) { _localPosition = v; m_dirty = true; }
    ),
    localRotation(
        [this]() { return _localRotation; },
        [this](Quaternion q) { _localRotation = q; m_dirty = true; }
    ),
    localScale(
        [this]() { return _localScale; },
        [this](Vector3 v) { _localScale = v; m_dirty = true; }
    ),
    position(
        // getter: グローバル座標
        [this]() {
            updateMatrices();
            return m_worldMatrix.translation();
        },
        // setter: グローバル座標からlocalPositionを逆算
        [this](Vector3 worldPos) {
            if (parent) {
                parent->updateMatrices();
                Matrix4x4 invParent = parent->m_worldMatrix.inverse();
                _localPosition = worldPos * invParent;
            }
            else {
                _localPosition = worldPos;
            }
            m_dirty = true;
        }
    ),
    rotation(
        [this]() {
            updateMatrices();
            // ワールド行列からクォータニオンを取得
            Vector3 s, t;
            Quaternion q;
            m_worldMatrix.Decompose(s, q, t);
            return q;
        },
        [this](Quaternion worldRot) {
            if (parent) {
                parent->updateMatrices();
                // 親のワールド回転の逆を掛けてローカル回転を算出
                Quaternion parentWorldRot, parentWorldRotInv;
                Vector3 s, t;
                parent->m_worldMatrix.Decompose(s, parentWorldRot, t);
                parentWorldRotInv = Inverse(parentWorldRot);
                _localRotation = worldRot * parentWorldRotInv;
            }
            else {
                _localRotation = worldRot;
            }
            m_dirty = true;
        }
    ),
    forward(
        // getter: ワールド空間の前方向
        [this]() {
            return TransformDirection(Vector3::forward);
        },
        // setter: worldForward に向くようワールド回転を設定
        [this](Vector3 worldForward) {
            if (worldForward.magnitude() < 1e-6f) return;
            Vector3 f = worldForward.normalized();

            // up が前方向とほぼ平行なら代替 up を使う
            Vector3 up = Vector3::up;
            if (std::abs(Dot(f, up)) > 0.999f) up = Vector3::right;

            // CreateWorld の引数は (position, forward, up)
            Matrix4x4 m = DirectX::SimpleMath::Matrix::CreateWorld(Vector3::zero, f, up);
            Vector3 s, t;
            Quaternion q;
            m.Decompose(s, q, t);
            rotation = q; // rotation プロパティの setter を使ってローカル回転を設定
        }
    ),
    up(
        // getter: ワールド空間の上方向
        [this]() {
            return TransformDirection(Vector3::up);
        },
        // setter: worldUp に向くようワールド回転を設定（可能な限り現在の forward を保持）
        [this](Vector3 worldUp) {
            if (worldUp.magnitude() < 1e-6f) return;
            Vector3 upVec = worldUp.normalized();

            // 現在の forward を取得（ワールド）
            Vector3 currF = TransformDirection(Vector3::forward);

            // 右方向を計算（forward x up）
            Vector3 right = Cross(currF, upVec);
            if (right.magnitude() < 1e-6f) {
                // forward と up がほぼ平行 -> 別の基準を使う
                currF = Vector3::forward;
                right = Cross(currF, upVec);
            }

            // 再計算した forward を正規化
            Vector3 f = Cross(upVec, right.normalized()).normalized();

            Matrix4x4 m = DirectX::SimpleMath::Matrix::CreateWorld(Vector3::zero, f, upVec);
            Vector3 s, t;
            Quaternion q;
            m.Decompose(s, q, t);
            rotation = q;
        }
    ),
    right(
        // getter: ワールド空間の右方向
        [this]() {
            return TransformDirection(Vector3::right);
        },
        // setter: worldRight に向くようワールド回転を設定（可能な限り現在の up を保持）
        [this](Vector3 worldRight) {
            if (worldRight.magnitude() < 1e-6f) return;
            Vector3 rVec = worldRight.normalized();

            // 現在の up を取得（ワールド）
            Vector3 currUp = TransformDirection(Vector3::up);

            // forward を計算 (up x right)
            Vector3 f = Cross(currUp, rVec);
            if (f.magnitude() < 1e-6f) {
                // up と right がほぼ平行 -> 別の基準を使う
                currUp = Vector3::up;
                f = Cross(currUp, rVec).normalized();
            }

            // 再計算した up を正規化
            Vector3 upVec = Cross(rVec, f).normalized();

            Matrix4x4 m = DirectX::SimpleMath::Matrix::CreateWorld(Vector3::zero, f, upVec);
            Vector3 s, t;
            Quaternion q;
            m.Decompose(s, q, t);
            rotation = q;
        }
    )
{
}

Transform::~Transform()
{
    for (auto& child : children)
    {
        if (child) child->transform->parent = nullptr;
    }
}

Vector3 Transform::TransformDirection(Vector3 localDirection) const
{
    // 平行移動成分を除外した回転・スケールのみ適用
    Matrix4x4 m = localToWorldMatrix();
    return m.MultiplyVector(localDirection);
}


// 親の変更
GameObject* Transform::SetParent(Transform * newParent)
{
    // 親のTransformから自分を外す
    if (parent == nullptr)
    {
        // 新規Transformに親を設定する場合はsmart_ptrを渡すstatic版を使ってください
        abort();
        return nullptr;
    }
    auto& siblings = parent->children;

    // 以前の親からGameObjectのスマートポインタを所有権ごと移動
    auto it = std::find_if(
        siblings.begin(), siblings.end(),
        [this](const unique_ptr<GameObject>& ptr) { return ptr->transform == this; });
    assert(it != siblings.end());

    GameObject* gameObject_ptr = it->get();
    assert(gameObject_ptr != nullptr);

    // 元の親から削除
    auto gameObject_owner = move(*it);
    siblings.erase(it);

    // 新しい親を設定
    parent = newParent;

    if (parent)
    {
        // 新しい親に自分を持つGameObjectを追加
        parent->children.push_back(std::move(gameObject_owner));
    }
    m_dirty = true;

    return gameObject_ptr;
}


void Transform::SetParent(unique_ptr<GameObject> gameObjectPtr, Transform* newParent)
{
    // 親のTransformから自分を外す
    if (gameObjectPtr->transform->parent != nullptr)
    {
        // すでに親がある場合はメンバ変数版を使ってください
        abort();
        return;
    }

    // 新しい親を設定
    gameObjectPtr->transform->parent = newParent;
    gameObjectPtr->transform->m_dirty = true;
    if (newParent)
    {
        // 新しい親に自分を持つGameObjectを追加
        newParent->children.push_back(std::move(gameObjectPtr));
    }
}


// 子を取得
Transform* Transform::GetChild(size_t index) const
{
    if (index < children.size() && children[index]) {
        return children[index]->transform;
    }
    return nullptr;
}

const Matrix4x4& Transform::localMatrix() const
{
    if (m_dirty) {
        m_localMatrix = DirectX::SimpleMath::Matrix::CreateScale(_localScale.x, _localScale.y, _localScale.z)
            * DirectX::SimpleMath::Matrix::CreateFromQuaternion(DirectX::XMFLOAT4(_localRotation))
            * DirectX::SimpleMath::Matrix::CreateTranslation(_localPosition.x, _localPosition.y, _localPosition.z);
    }
    return m_localMatrix;
}


// 行列を更新
void Transform::updateMatrices() const
{
    if (parent) {
        parent->updateMatrices();
    }

    if (m_dirty)
    {
        m_localMatrix = DirectX::SimpleMath::Matrix::CreateScale(_localScale)
            * DirectX::SimpleMath::Matrix::CreateFromQuaternion(DirectX::XMFLOAT4(_localRotation))
            * DirectX::SimpleMath::Matrix::CreateTranslation(_localPosition);
        if (parent) {
            m_worldMatrix = m_localMatrix * parent->m_worldMatrix;
        }
        else {
            m_worldMatrix = m_localMatrix;
        }
        m_dirty = false;

        // 親の行列が変わったので、子も変わるように
        for (auto& c : children)
        {
            c->transform->m_dirty = true;
        }
    }
}


}
