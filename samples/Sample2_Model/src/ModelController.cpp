#include "ModelController.h"

#include <UniDx/Input.h>


using namespace DirectX;
using namespace UniDx;


void ModelController::OnEnable()
{
    pitch = -10.0f;
    yaw = 0.0f;
}


void ModelController::Update()
{
    const float angleSpeed = 120.0f;

    // WASDによってピッチ角とヨー角を変える
    Quaternion rot;
    if (Input::GetKey(Keyboard::A))
    {
        yaw += angleSpeed * Time::deltaTime;
    }
    if (Input::GetKey(Keyboard::D))
    {
        yaw -= angleSpeed * Time::deltaTime;
    }
    if (Input::GetKey(Keyboard::W))
    {
        pitch += angleSpeed * Time::deltaTime;
    }
    if (Input::GetKey(Keyboard::S))
    {
        pitch -= angleSpeed * Time::deltaTime;
    }

    // ローカルの方向Quaternionに乗算する
    transform->localRotation = Quaternion::Euler(pitch, yaw, 0.0f);
}
