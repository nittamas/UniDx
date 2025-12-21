#pragma once

#include "Behaviour.h"


namespace UniDx {

// --------------------
// Cameraクラス
// --------------------
class Camera : public Behaviour
{
public:
    static Camera* main;

    float fov = 60.0f;
    float nearClip = 0.1f;
    float farClip = 1000.0f;

    Matrix4x4 GetViewMatrix() const;

    Matrix4x4 GetProjectionMatrix(float aspect) const;

    // 定数バッファ更新
    void UpdateConstantBuffer();

protected:
    virtual void OnEnable() override;
    virtual void OnDisable() override;

    ComPtr<ID3D11Buffer> constantBufferPerCamera; // カメラごとにGPUで共通利用する定数バッファ
};

} // namespace UniDx
