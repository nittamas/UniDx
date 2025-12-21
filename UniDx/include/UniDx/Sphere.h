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
// Sphereクラス
// --------------------
struct Sphere : public DirectX::BoundingSphere
{
public:
    Sphere() = default;

    Sphere(Vector3 center, float radius)
        : DirectX::BoundingSphere(center, radius) {}
};

} // namespace UniDx
