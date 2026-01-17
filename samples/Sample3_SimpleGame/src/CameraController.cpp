#include "CameraController.h"

#include <UniDx/Input.h>
#include <UniDx/Time.h>

#include "Player.h"


using namespace DirectX;
using namespace UniDx;

void CameraController::OnEnable()
{
    Quaternion rot = Quaternion::AngleAxis(35, Vector3::right);
    transform->localRotation = rot;

    // 配置位置から角度と距離を記録
    auto dir = player->transform->position - transform->position;
    float planar = std::sqrt(dir.x * dir.x + dir.z * dir.z);
    length = dir.magnitude();
    yaw = std::atan2(dir.x, dir.z) * Rad2Deg;
    pitch = std::abs(std::atan2(dir.y, planar) * Rad2Deg);
}


void CameraController::Update()
{
    // 操作に応じて角度を変更
    const float angleSpeed = 90.0f;
    if (Input::GetKey(Keyboard::J))
    {
        yaw += angleSpeed * Time::deltaTime;
    }
    if (Input::GetKey(Keyboard::L))
    {
        yaw -= angleSpeed * Time::deltaTime;
    }
    if (Input::GetKey(Keyboard::I))
    {
        pitch = std::min(pitch + angleSpeed * Time::deltaTime, 90.0f);
    }
    if (Input::GetKey(Keyboard::K))
    {
        pitch = std::max(pitch - angleSpeed * Time::deltaTime, 0.0f);
    }
    Quaternion rot = Quaternion::Euler(pitch, yaw, 0.0f);
    transform->rotation = rot;
    transform->position = (Vector3::forward * rot) * -length + player->transform->position;
}
