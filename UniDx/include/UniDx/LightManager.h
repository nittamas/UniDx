#pragma once

#include <vector>
#include <array>

#include "UniDxDefine.h"
#include "Singleton.h"
#include "ConstantBuffer.h"


// UniDxではシェーダ簡易化のため、一番影響の大きいディレクショナルライト１つ、
// ポイントライト最大８個、スポットライト最大８個をそれぞれ転送します。

namespace UniDx
{

class Light;

struct GPULight // 16byte aligned
{
    Vector3 positionOrDirWS;
    float rangeOrInvCos;
    Color color;
    Vector3 spotDirWS;
    float spotOuterCos;
    uint32_t  type;
    uint32_t shadowIndex;
    uint32_t  padding[2];
};


// --------------------
// LightManager
// --------------------
class LightManager : public Singleton<LightManager>
{
public:
    static constexpr int LightCountMax = 32;

    Color ambientColor;

    LightManager();

    bool registerLight(Light* light);
    void unregisterLight(Light* light);

    // ライト情報を定数バッファに反映
    virtual void updateLightCBuffer();
    virtual void updateLightCBufferObject(Vector3 objPos, int lightCountMax = PointLightCountMax + SpotLightCountMax);

private:
    std::vector<Light*> lights_;
    size_t              capacity_ = 0;

    std::vector<GPULight> gpuLights_;
    Microsoft::WRL::ComPtr<ID3D11Buffer>           lightBuf_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>lightSRV_;
//    Microsoft::WRL::ComPtr<ID3D11Buffer>           metaCB_;

    std::vector<PointLightBuffer> pointLights;
    std::vector<SpotLightBuffer> spotLights;
    std::vector<float> pointLightIntensity;
    std::vector<float> spotLightIntensity;
    ComPtr<ID3D11Buffer> constantBufferLightPerFrame;  // フレームごとにGPUで共通利用する定数バッファ
    ComPtr<ID3D11Buffer> constantBufferLightPerObject; // オブジェクトごとにGPUで共通利用する定数バッファ
};

}
