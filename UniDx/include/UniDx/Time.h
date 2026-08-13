#pragma once

#include "Property.h"

namespace UniDx
{

// Time情報
class Time
{
public:
    /** @brief 開始からの経過フレーム数 */
    static inline int frameCount = 0;

    /** @brief 固定時間ステップ */
	static inline float fixedDeltaTime = 0.01667f;

    /** @brief 最大許容時間ステップ */
    static inline float maximumDeltaTime = 0.2f;

    /** @brief 現在の時間（timeScaleを考慮した値） */
    static inline float time = 0.0f;

    /** @brief 時間のスケール（1.0fが通常速度、0.5fが半分の速度、2.0fが倍速） */
    static inline float timeScale = 1.0f;

    /** @brief timeScaleを考慮しない現在の時間 */
    static inline float unscaledTime = 0.0f;

    /** @brief timeScaleを考慮しない前フレームからの経過時間 */
    static inline float unscaledDeltaTime = 0.0f;

    /** @brief 前フレームからの経過時間（timeScaleを考慮した値） */
    static inline ReadOnlyProperty<float> deltaTime = ReadOnlyProperty<float>([]() { return unscaledDeltaTime * timeScale; });

    static void Start()
    {
        frameCount = 0;
        time = 0.0f;
        timeScale = 1.0f;
    }

    static void SetDeltaTimeFixed()
    {
        unscaledDeltaTime = fixedDeltaTime;
    }

    static void SetDeltaTimeFrame()
    {
        unscaledDeltaTime = float(realDeltaTime);
    }

    static void UpdateFrame(double rt)
    {
        realDeltaTime = rt;
        frameCount++;
        time = float(time + rt * timeScale);
        unscaledTime += float(rt);
    }

private:
    static inline double realDeltaTime;
};

}
