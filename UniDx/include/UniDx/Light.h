#pragma once

#include "Behaviour.h"


namespace UniDx
{

using namespace std;

enum LightType
{
    LightType_Spot,
    LightType_Directional,
    LightType_Point,
};



// --------------------
// Lightクラス
// --------------------
class Light : public Behaviour
{
public:
    Color color;
    LightType type;
    float intensity;
    float range;
    float spotAngle;    // スポットライトの角度°

    Light() : color(1,1,1,1), type(LightType_Directional), intensity(1), range(100), spotAngle(90){}

protected:
    virtual void OnEnable() override;
    virtual void OnDisable() override;
};

} // namespace UniDx
