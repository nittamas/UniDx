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

    Bounds() noexcept = default;

    Bounds(Vector3 center, Vector3 extents) noexcept : Center(center), extents(extents)  {}

    /** @brief サイズ（全長）*/
    constexpr Vector3 size() const noexcept { return Vector3(extents.x * 2, extents.y * 2, extents.z * 2); }

    /** @brief 最小座標*/
    constexpr Vector3 min() const noexcept { return Vector3(Center) - extents; }

    /** @brief 最大座標*/
    constexpr Vector3 max() const noexcept { return Vector3(Center) + extents; }

    /** @brief 指定点に最も近い点*/
    [[nodiscard]] Vector3 ClosestPoint(Vector3 point) const noexcept {
        Vector3 mn = min();
        Vector3 mx = max();
        return Vector3(
            std::max(mn.x, std::min(point.x, mx.x)),
            std::max(mn.y, std::min(point.y, mx.y)),
            std::max(mn.z, std::min(point.z, mx.z))
        );
    }

    /** @brief Boundsを拡張（全体サイズを増やす）*/
    void Expand(float amount) noexcept {
        Vector3 delta(amount, amount, amount);
        extents = extents + delta * 0.5f;
    }
    /** @brief Boundsを拡張（全体サイズを増やす）*/
    void Expand(Vector3 amount) noexcept {
        extents = extents + amount * 0.5f;
    }

    /** @brief 指定点を含むように拡張*/
    void Encapsulate(Vector3 point) noexcept {
        Vector3 mn = min();
        Vector3 mx = max();
        SetMinMax(Min(mn, point), Max(mx, point));
    }
    /** @brief 指定Boundsを含むように拡張*/
    void Encapsulate(const Bounds& bounds) noexcept {
        Encapsulate(bounds.min());
        Encapsulate(bounds.max());
    }

    /** @brief min/maxからBoundsを再設定*/
    void SetMinMax(Vector3 min, Vector3 max) noexcept {
        Center = (min + max) * 0.5f;
        extents = (max - min) * 0.5f;
    }

    /** @brief 他のBoundsと交差しているか*/
    [[nodiscard]] bool Intersects(const Bounds& bounds) const noexcept {
        if(std::abs(Center.x - bounds.Center.x) > extents.x + bounds.extents.x) return false;
        if(std::abs(Center.z - bounds.Center.z) > extents.z + bounds.extents.z) return false;
        if(std::abs(Center.y - bounds.Center.y) > extents.y + bounds.extents.y) return false;
        return true;
    }

    /** @brief 指定点までの二乗距離*/
    float SqrDistance(Vector3 point) const noexcept {
        Vector3 cp = ClosestPoint(point);
        Vector3 d = cp - point;
        return d.sqrMagnitude();
    }
};

} // namespace UniDx
