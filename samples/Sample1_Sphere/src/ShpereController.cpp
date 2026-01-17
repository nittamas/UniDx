#include "ShpereController.h"

#include <UniDx/Input.h>


using namespace DirectX;
using namespace UniDx;


void ShpereController::Update()
{
    const float angleSpeed = 90.0f;

    // WASDによって回転するQuaternionを作る
    Quaternion rot;
    if (Input::GetKey(Keyboard::A))
    {
        rot = rot * Quaternion::AngleAxis(-angleSpeed * Time::deltaTime, Vector3::up);
    }
    if (Input::GetKey(Keyboard::D))
    {
        rot = rot * Quaternion::AngleAxis(angleSpeed * Time::deltaTime, Vector3::up);
    }
    if (Input::GetKey(Keyboard::W))
    {
        rot = rot * Quaternion::AngleAxis(-angleSpeed * Time::deltaTime, Vector3::right);
    }
    if (Input::GetKey(Keyboard::S))
    {
        rot = rot * Quaternion::AngleAxis(angleSpeed * Time::deltaTime, Vector3::right);
    }

    // ローカルの方向Quaternionに乗算する
    transform->localRotation = rot * transform->localRotation;
}
