#include "LightController.h"

#include <UniDx/Input.h>

using namespace UniDx;


void LightController::OnEnable()
{
    // 配置位置から距離と角度を記録
    auto pos = transform->localPosition.get();
    float planar = std::sqrt(pos.x * pos.x + pos.z * pos.z);
    length = pos.magnitude();
    yaw = std::atan2(pos.x, pos.z) * Rad2Deg + 180.0f;
    pitch = std::atan2(pos.y, planar) * Rad2Deg * 1.0f;
}


void LightController::Update()
{
    // 操作に応じて角度を変更
    const float angleSpeed = 90.0f;
    if (Input::GetKey(Keyboard::O))
    {
        yaw += angleSpeed * Time::deltaTime;
    }
    if (Input::GetKey(Keyboard::P))
    {
        yaw -= angleSpeed * Time::deltaTime;
    }
    transform->localRotation = Quaternion::Euler(pitch, yaw, 0.0f);
    transform->localPosition = transform->forward * -length;
}
