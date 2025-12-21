#include "pch.h"
#include <UniDx/Camera.h>
#include <UniDx/ConstantBuffer.h>
#include <UniDx/D3DManager.h>
#include <SimpleMath.h>


namespace UniDx{

// static なメインカメラ
Camera* Camera::main;

Matrix4x4 Camera::GetViewMatrix() const
{
    // ワールド行列を分解
    Vector3 scale, translation;
    Quaternion rotation;
    Matrix4x4 mtx = transform->getLocalToWorldMatrix();
    mtx.Decompose(scale, rotation, translation);

    // スケールを(1,1,1)にして再構成
    Matrix4x4 worldNoScale = Matrix4x4::Rotate(rotation) * Matrix4x4::Translate(translation);

    // 逆行列を返す（ビュー行列）
    return worldNoScale.inverse();
}

Matrix4x4 Camera::GetProjectionMatrix(float aspect) const
{
    using namespace DirectX;
    return Matrix4x4(DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), aspect, nearClip, farClip));
}


void Camera::UpdateConstantBuffer()
{
    // 時間に関わる time, unscaledDeltaTime, 1/unscaledDeltaTime, frameCount を送信
    constexpr float minDt = 1.0f / 600.0f;
    float dt = std::max(Time::unscaledDeltaTime, minDt);

    ConstantBufferPerCamera cb{};
    cb.view = GetViewMatrix();
    cb.projection = GetProjectionMatrix(16.0f / 9.0f);
    cb.cameraPosW = transform->position;
    cb.cameraNear = nearClip;
    cb.cameraForwardW = transform->forward;
    cb.cameraFar = farClip;
    cb.time.x = Time::time;
    cb.time.y = dt;
    cb.time.z = 1.0f / dt;
    cb.time.w = float(Time::frameCount);
    D3DManager::getInstance()->GetContext()->UpdateSubresource(constantBufferPerCamera.Get(), 0, nullptr, &cb, 0, 0);

    // 定数バッファ更新
    ID3D11Buffer* cbs[1] = { constantBufferPerCamera.Get() };
    D3DManager::getInstance()->GetContext()->VSSetConstantBuffers(CB_PerCamera, 1, cbs);
    D3DManager::getInstance()->GetContext()->PSSetConstantBuffers(CB_PerCamera, 1, cbs);
}


void Camera::OnEnable()
{
    if (main == nullptr)
    {
        main = this;
    }

    // フレームごとにGPUで共通利用する定数バッファ
    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth = sizeof(ConstantBufferPerCamera);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    D3DManager::getInstance()->GetDevice()->CreateBuffer(&desc, nullptr, constantBufferPerCamera.GetAddressOf());
}


void Camera::OnDisable()
{
    if (main == this)
    {
        main = nullptr;
    }
}

}
