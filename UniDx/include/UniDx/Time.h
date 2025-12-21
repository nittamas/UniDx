#pragma once

#include "Property.h"

namespace UniDx
{

// Time情報
class Time
{
public:
    static inline int frameCount = 0;

    static inline float fixedDeltaTime = 0.01667f;

    static inline float time = 0.0f;

    static inline float timeScale = 1.0f;

    static inline float unscaledTime = 0.0f;

    static inline float unscaledDeltaTime = 0.0f;

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
