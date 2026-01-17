#pragma once

#include "UniDxDefine.h"
#include "BoneMath.h"

namespace UniDx {

// スキンメッシュに使うボーンの最大値
constexpr int SkinMeshBoneMax = 256;

// UniDxで使用する定数バッファのスロット番号
// すべてのシェーダーはこの番号に従って指定する
enum CbSlot : uint8_t
{
    CB_UserStart = 0,        // ゲーム用バッファの開始番号
    CB_User0 = CB_UserStart,
    CB_User1 = CB_UserStart + 1,
    CB_User2 = CB_UserStart + 2,
    CB_User3 = CB_UserStart + 3,

    CB_PerCamera = 8,       // カメラごと
    CB_PerObject = 9,       // オブジェクトごと
    CB_PerMaterial = 10,    // マテリアルごと
    CB_LightPerFrame = 11,  // フレーム共通ライト用
    CB_LightPerObject = 12  // オブジェクトごとライト用
};


// -----------------------------------------------------------------------------
// シェーダー側と共有する、カメラと時間の定数バッファ
// -----------------------------------------------------------------------------
struct ConstantBufferPerCamera
{
    Matrix4x4 view;
    Matrix4x4 projection;
    Vector3   cameraPosW;
    float     cameraNear;
    Vector3   cameraForwardW;
    float     cameraFar;
    Vector4   time;       // (t, dt, 1/dt, frameCount)
};
/* HLSL
cbuffer CBPerCamera : register(b8)
{
    row_major float4x4 view;
    row_major float4x4 projection;
    float3   cameraPosW;
    float    cameraNear;
    float3   cameraForwardW;
    float    cameraFar;
    float4   time; // (t, dt, 1/dt, frameCount)
};
*/

// -----------------------------------------------------------------------------
// シェーダー側と共有する、モデルビュー行列の定数バッファ
// -----------------------------------------------------------------------------
struct ConstantBufferPerObject
{
    Matrix4x4 world;
};
/* HLSL
cbuffer CBPerObject : register(b9)
{
    row_major float4x4 world;
};
*/
struct ConstantBufferSkinPerObject
{
    Matrix4x4 world;
    BoneMat3x4 bones[SkinMeshBoneMax];
};
/* HLSL
cbuffer CBSkinPerObject : register(b9)
{
    row_major float4x4 world;
    BoneMat3x4 bones[256];
};
*/

// -----------------------------------------------------------------------------
// シェーダー側と共有する、マテリアルの定数バッファ
// -----------------------------------------------------------------------------
struct ConstantBufferPerMaterial
{
    Color baseColor;
};
/* HLSL
cbuffer CBPerMaterial : register(b10)
{
    float4 baseColor;
};
*/

// -----------------------------------------------------------------------------
// シェーダー側と共有する、ワールドライトの定数バッファ
// -----------------------------------------------------------------------------
struct ConstantBufferLightPerFrame
{
    Color ambientColor;     // rgb=Color, a=Intensity
    Color directionalColor; // rgb=Color, a=Intensity
    Vector3 directionW;
    uint32_t pad;
};
/* HLSL
cbuffer CBLightPerFrame : register(b11)
{
    float4 ambientColor;
    float4 directionalColor;
    float3 directionW;
    uint pad;
};
*/

// -----------------------------------------------------------------------------
// シェーダー側と共有する、オブジェクトごとのライトの定数バッファ
// -----------------------------------------------------------------------------
static constexpr int PointLightCountMax = 8;
static constexpr int SpotLightCountMax = 8;
struct PointLightBuffer
{
    Color color;        // rgb=Color, a=Intensity
    Vector3 positionW;
    float rangeInv;
};
struct SpotLightBuffer
{
    Color color;        // rgb=Color, a=Intensity
    Vector3 positionW;
    float rangeInv;
    Vector3 directionW;
    float outerCos;
};
struct ConstantBufferLightPerObject
{
    PointLightBuffer    pointLights[PointLightCountMax];
    SpotLightBuffer     spotLights[SpotLightCountMax];
    uint32_t            pointLightCount;
    uint32_t            spotLightCount;
    uint32_t            pad[2];
};
/* HLSL
struct PointLight
{
    float4 color;
    float3 positionW;
    float rangeInv;
};
struct SpotLight
{
    float4 color;
    float3 positionW;
    float rangeInv;
    float3 directionW;
    float outerCos;
};
cbuffer CBLightPerObject : register(b12)
{
    PointLight pointLights[8]; // 最大8個
    SpotLight spotLights[8];   // 最大8個
    uint pointLightCount;
    uint spotLightCount;
    float2 _pad; // 16byte 境界合わせ
};
*/

}