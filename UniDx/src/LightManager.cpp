#include "pch.h"
#include <UniDx/LightManager.h>

#include <algorithm>
#include <UniDx/Light.h>
#include <UniDx/D3DManager.h>


namespace UniDx
{

using namespace std;


// 指定位置に対するライトの影響の強さを計算
// α値の強さと距離減衰を使う
float getLightIntensity(Vector3 position, const PointLightBuffer& light)
{
    return light.color.a * (1.0f - (position - light.positionW).magnitude() * light.rangeInv);
}
float getLightIntensity(Vector3 position, const SpotLightBuffer& light)
{
    return light.color.a * (1.0f - (position - light.positionW).magnitude() * light.rangeInv);
}

// ソート済みのTのvectorと比較floatのvectorに対して、比較floatを追加してソート
template<typename T>
void addSort(vector<T>& valueVec, vector<float>& compVec, float comp)
{
    compVec.push_back(comp);
    assert(valueVec.size() >= compVec.size());
    for (size_t i = compVec.size() - 1; i >= 1; i--)
    {
        if (compVec[i] > compVec[i - 1])
        {
            std::swap(compVec[i], compVec[i - 1]);
            std::swap(valueVec[i], valueVec[i - 1]);
        }
        else
        {
            return; // 終了
        }
    }
}

// コンストラクタ
LightManager::LightManager()
{
    // 環境光
    ambientColor = Color(0.2f, 0.2f, 0.2f, 1.0f);

    // ライトのリザーブ
    pointLights.reserve(PointLightCountMax);
    spotLights.reserve(SpotLightCountMax);

    // 定数バッファ作成
    D3D11_BUFFER_DESC desc{};
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;

    desc.ByteWidth = sizeof(ConstantBufferLightPerFrame);
    D3DManager::getInstance()->GetDevice()->CreateBuffer(&desc, nullptr, constantBufferLightPerFrame.GetAddressOf());

    desc.ByteWidth = sizeof(ConstantBufferLightPerObject);
    D3DManager::getInstance()->GetDevice()->CreateBuffer(&desc, nullptr, constantBufferLightPerObject.GetAddressOf());
}


// Lightを登録
bool LightManager::registerLight(Light* light)
{
    for (size_t i = 0; i < lights_.size(); ++i)
    {
        if (lights_[i] == nullptr)
        {
            lights_[i] = light;
            return true;
        }
        if (lights_[i] == light)
        {
            return true; // 登録済み
        }
    }

    // 無効化されたものがなければ追加
    if (lights_.size() < LightCountMax)
    {
        lights_.push_back(light);
        return true;
    }
    return false;
}


// Lightの登録を解除
void LightManager::unregisterLight(Light* light)
{
    for (size_t i = 0; i < lights_.size(); ++i)
    {
        if (lights_[i] == light)
        {
            lights_[i] = light;
            return;
        }
    }
}


// ライト情報を定数バッファに反映
void LightManager::updateLightCBuffer()
{
    // 無効になっているものをvectorから削除
    for (vector<Light*>::iterator it = lights_.begin(); it != lights_.end();)
    {
        if (*it == nullptr)
        {
            it = lights_.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // 定数バッファ更新
    ConstantBufferLightPerFrame cb{};
    cb.ambientColor = ambientColor;
    cb.directionalColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
    cb.directionW = Vector3::forward;

    float maxIntensity = 0.0f;
    for (vector<Light*>::iterator it = lights_.begin(); it != lights_.end(); ++it)
    {
        (*it)->color.a = (*it)->intensity; // 全ライトのaに強さを入れる

        if ((*it)->type == LightType_Directional && (*it)->color.a >= maxIntensity)
        {
            cb.directionalColor = (*it)->color;
            cb.directionW = (*it)->transform->forward;
        }
    }
    D3DManager::getInstance()->GetContext()->UpdateSubresource(constantBufferLightPerFrame.Get(), 0, nullptr, &cb, 0, 0);

    ID3D11Buffer* cbs[1] = { constantBufferLightPerFrame.Get() };
    D3DManager::getInstance()->GetContext()->PSSetConstantBuffers(CB_LightPerFrame, 1, cbs);
}

void LightManager::updateLightCBufferObject(Vector3 objPos, int lightCountMax)
{
    int pointLightMax = std::clamp(lightCountMax, 0, PointLightCountMax);
    int spotLightMax = std::clamp(lightCountMax, 0, SpotLightCountMax);

    pointLights.clear();
    spotLights.clear();
    pointLightIntensity.clear();
    spotLightIntensity.clear();

    for (Light* l : lights_)
    {
        auto world = l->transform->getLocalToWorldMatrix();       // ワールド行列取得
        float rangeInv = l->range != 0.0f ? 1.0f / l->range : 0.0f;

        bool popPoint = false; // ポイントライトの削除が必要か
        bool popSpot = false; // スポットライトの削除が必要か
        switch (l->type)
        {
        // ポイントライトの追加
        case LightType_Point:
            pointLights.emplace_back(l->color, l->transform->position, rangeInv);
            if (pointLights.size() >= pointLightMax)
            {
                for (size_t i = pointLightIntensity.size(); i < pointLightMax; ++i)
                {
                    addSort(pointLights, pointLightIntensity, getLightIntensity(objPos, pointLights[i]));
                }
                popPoint = true;
            }
            break;

        // スポットライトの追加
        case LightType_Spot:
            spotLights.emplace_back(l->color,
                l->transform->position, rangeInv,
                l->transform->forward, cosf(DirectX::XMConvertToRadians(l->spotAngle * 0.5f))
            );
            if (spotLights.size() >= spotLightMax)
            {
                for (size_t i = spotLightIntensity.size(); i < spotLightMax; ++i)
                {
                    addSort(spotLights, spotLightIntensity, getLightIntensity(objPos, spotLights[i]));
                }
                popSpot = true;
            }
            break;
        }

        // 合計数で超えたら、影響度の少ないほうを裂くk所
        if(!popPoint && !popSpot && pointLights.size() + spotLights.size() > lightCountMax)
        {
            for (size_t i = pointLightIntensity.size(); i < pointLightMax; ++i)
            {
                addSort(pointLights, pointLightIntensity, getLightIntensity(objPos, pointLights[i]));
            }
            for (size_t i = spotLightIntensity.size(); i < spotLightMax; ++i)
            {
                addSort(spotLights, spotLightIntensity, getLightIntensity(objPos, spotLights[i]));
            }
            if (spotLights.size() == 0 ||
                pointLights.size() > 0 && pointLightIntensity.back() < spotLightIntensity.back())
            {
                popPoint = true;
            }
            else
            {
                popSpot = true;
            }
        }

        if (popPoint)
        {
            pointLights.pop_back();
            pointLightIntensity.pop_back();
        }
        else if (popSpot)
        {
            spotLights.pop_back();
            spotLightIntensity.pop_back();
        }
    }

    // 定数バッファ更新
    ConstantBufferLightPerObject cb{};
    cb.pointLightCount = uint32_t(pointLights.size());
    std::copy(pointLights.begin(), pointLights.end(), cb.pointLights);
    cb.spotLightCount = uint32_t(spotLights.size());
    std::copy(spotLights.begin(), spotLights.end(), cb.spotLights);
    D3DManager::getInstance()->GetContext()->UpdateSubresource(constantBufferLightPerObject.Get(), 0, nullptr, &cb, 0, 0);

    ID3D11Buffer* cbs[1] = { constantBufferLightPerObject.Get() };
    D3DManager::getInstance()->GetContext()->PSSetConstantBuffers(CB_LightPerObject, 1, cbs);
}


} // UniDx
