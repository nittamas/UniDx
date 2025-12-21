#include "pch.h"
#include <UniDx/AnimationCurve.h>

namespace UniDx
{

// Hermite補間関数
static float Hermite(float p0, float m0, float p1, float m1, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;
    float h00 = 2 * t3 - 3 * t2 + 1;
    float h10 = t3 - 2 * t2 + t;
    float h01 = -2 * t3 + 3 * t2;
    float h11 = t3 - t2;
    return h00 * p0 + h10 * m0 + h01 * p1 + h11 * m1;
}

float AnimationCurve::Evaluate(float time) const
{
    if (keyframes.empty()) return 0.0f;
    if (time <= keyframes.front().time) return keyframes.front().value;
    if (time >= keyframes.back().time) return keyframes.back().value;

    // キーフレーム間を探索
    for (size_t i = 0; i < keyframes.size() - 1; ++i)
    {
        const Keyframe& k0 = keyframes[i];
        const Keyframe& k1 = keyframes[i + 1];
        if (time >= k0.time && time <= k1.time)
        {
            float dt = k1.time - k0.time;
            float t = (time - k0.time) / dt;
            float m0 = k0.outTangent * dt;
            float m1 = k1.inTangent * dt;
            return Hermite(k0.value, m0, k1.value, m1, t);
        }
    }
    return 0.0f;
}

}