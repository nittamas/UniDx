#pragma once
#include <string>

#include "UniDxDefine.h"
#include "Property.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace UniDx {

// --------------------
// Boundsクラス
// --------------------
struct Bounds
{
public:
    Vector3 Center;            // ボックスの中心
    Vector3 extents;           // 中心からコーナー

    Bounds() = default;

    Bounds(Vector3 center, Vector3 extents) : Center(center), extents(extents) {}

    // サイズ（全長）
    Vector3 size() const { return Vector3(extents.x * 2, extents.y * 2, extents.z * 2); }

    // 最小座標
    Vector3 min() const { return Vector3(Center) - extents; }

    // 最大座標
    Vector3 max() const { return Vector3(Center) + extents; }

    // 指定点に最も近い点
    Vector3 ClosestPoint(Vector3 point) const {
        Vector3 mn = min();
        Vector3 mx = max();
        return Vector3(
            std::max(mn.x, std::min(point.x, mx.x)),
            std::max(mn.y, std::min(point.y, mx.y)),
            std::max(mn.z, std::min(point.z, mx.z))
        );
    }

    // Boundsを拡張（全体サイズを増やす）
    void Expand(float amount) {
        Vector3 delta(amount, amount, amount);
        extents = extents + delta * 0.5f;
    }
    void Expand(Vector3 amount) {
        extents = extents + amount * 0.5f;
    }

    // 指定点を含むように拡張
    void Encapsulate(Vector3 point) {
        Vector3 mn = min();
        Vector3 mx = max();
        mn.x = std::min(mn.x, point.x);
        mn.y = std::min(mn.y, point.y);
        mn.z = std::min(mn.z, point.z);
        mx.x = std::max(mx.x, point.x);
        mx.y = std::max(mx.y, point.y);
        mx.z = std::max(mx.z, point.z);
        SetMinMax(mn, mx);
    }
    // 指定Boundsを含むように拡張
    void Encapsulate(const Bounds& bounds) {
        Encapsulate(bounds.min());
        Encapsulate(bounds.max());
    }

    // min/maxからBoundsを再設定
    void SetMinMax(Vector3 min, Vector3 max) {
        Center = (min + max) * 0.5f;
        extents = (max - min) * 0.5f;
    }

    // 他のBoundsと交差しているか
    bool Intersects(const Bounds& bounds) const {
        if(std::abs(Center.x - bounds.Center.x) > extents.x + bounds.extents.x) return false;
        if(std::abs(Center.y - bounds.Center.y) > extents.y + bounds.extents.y) return false;
        if(std::abs(Center.z - bounds.Center.z) > extents.z + bounds.extents.z) return false;
        return true;
    }

    // 指定点までの二乗距離
    float SqrDistance(Vector3 point) const {
        Vector3 cp = ClosestPoint(point);
        Vector3 d = cp - point;
        return d.sqrMagnitude();
    }
};

} // namespace UniDx
