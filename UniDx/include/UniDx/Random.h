#pragma once

#include <chrono>
#include "UniDxDefine.h"


namespace UniDx
{

// Random
class Random
{
public:
    // シングルトン的に使う場合のグローバルインスタンス
    static Random& global()
    {
        static Random inst;
        return inst;
    }

    explicit Random(uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count())
    {
        InitState(seed);
    }

    // シード設定 (Unity互換: InitState)
    void InitState(uint64_t seed)
    {
        state = seed ? seed : 88172645463325252ull;
    }

    // シード値の取得
    uint64_t getState() const { return state; }

    // 0.0～1.0の乱数（1.0を含む、Unity互換）
    float value()
    {
        // 24bit乱数を[0,1]に正規化、1.0を含む
        uint32_t v = static_cast<uint32_t>(nextUInt64() >> 40);
        return static_cast<float>(v) / static_cast<float>(0xFFFFFF); // 0xFFFFFF == 2^24-1
    }

    // [min, max] のfloat乱数（最大値含む、Unity互換）
    float Range(float min, float max)
    {
        // value()は1.0を含むのでそのまま
        return min + (max - min) * value();
    }

    // [min, max] のint乱数（最大値含む、Unityと異なる挙動に注意！）
    int Range(int min, int max)
    {
        return min + static_cast<int>(nextUInt64() % (static_cast<uint64_t>(max - min + 1)));
    }

    // [min, max) のfloat乱数（最大値含まない）
    float RangeExclusive(float min, float maxExclusive)
    {
        return min + (maxExclusive - min) * value();
    }

    // [min, max) のint乱数（最大値含まない）
    int RangeExclusive(int min, int maxExclusive)
    {
        return min + static_cast<int>(nextUInt64() % static_cast<uint64_t>(maxExclusive - min));
    }

    // -1.0～1.0のfloat乱数
    float symmetricValue()
    {
        return value() * 2.0f - 1.0f;
    }

    // 単位球内のランダムな点
    Vector3 insideUnitSphere()
    {
        while (true)
        {
            float x = symmetricValue();
            float y = symmetricValue();
            float z = symmetricValue();
            float len2 = x * x + y * y + z * z;
            if (len2 <= 1.0f)
                return Vector3(x, y, z);
        }
    }

    // 単位円内のランダムな点
    Vector2 insideUnitCircle()
    {
        while (true)
        {
            float x = symmetricValue();
            float y = symmetricValue();
            float len2 = x * x + y * y;
            if (len2 <= 1.0f)
                return Vector2(x, y);
        }
    }

    // 単位球面上のランダムな点（表面上、均一分布）
    Vector3 onUnitSphere()
    {
        while(true)
        {
            float x = symmetricValue();
            float y = symmetricValue();
            float z = symmetricValue();
            float len2 = x * x + y * y + z * z;
            if (len2 <= 1.0f && len2 > 0.0f)
            {
                float invLen = 1.0f / std::sqrt(len2);
                return Vector3(x * invLen, y * invLen, z * invLen);
            }
        }
    }

    // 一様分布の回転（単位四元数）を返す
    Quaternion rotationUniform()
    {
        // Shoemake のアルゴリズムに基づく一様な単位四元数生成
        // u1, u2, u3 を [0,1] から取り、以下の変換で一様な方向を得る
        float u1 = value();
        float u2 = value();
        float u3 = value();

        float sqrt1MinusU1 = std::sqrt(1.0f - u1);
        float sqrtU1 = std::sqrt(u1);

        float theta1 = PI * 2 * u2;
        float theta2 = PI * 2 * u3;

        float x = sqrt1MinusU1 * std::sinf(theta1);
        float y = sqrt1MinusU1 * std::cosf(theta1);
        float z = sqrtU1 * std::sinf(theta2);
        float w = sqrtU1 * std::cosf(theta2);

        return Quaternion(x, y, z, w);
    }

private:
    uint64_t state = 88172645463325252ull; // デフォルトシード

    // 64bit XorShift
    uint64_t nextUInt64()
    {
        uint64_t x = state;
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        state = x;
        return x;
    }
};


}
