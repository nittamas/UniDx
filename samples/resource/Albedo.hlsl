// ----------------------------------------------------------
// 頂点
// ----------------------------------------------------------
// カメラ定数バッファ
cbuffer CBPerCamera : register(b8)
{
    float4x4 view;
    float4x4 projection;
    float3 cameraPosW;
    float cameraNear;
    float3 cameraForwardW;
    float cameraFar;
    float4 time; // (t, dt, 1/dt, frameCount)
};

// オブジェクト定数バッファ
cbuffer CBPerObject : register(b9)
{
    float4x4 world;
};

// 頂点シェーダーへ入力するデータ
struct VSInput
{
    float3 pos : POSITION;
    float2 uv : TEXUV;
};


// 頂点シェーダーから出力するデータ
struct PSInput
{
    float4 pos : SV_Position; // 頂点の座標(射影座標系)
    float2 uv : TEXCOORD0; // UV座標
};

// 頂点シェーダー
PSInput VS(VSInput vin)
{
    PSInput Out;
    float4 p = float4(vin.pos.xyz, 1);
    p = mul(world, p);
    p = mul(view, p);
    p = mul(projection, p);
    Out.pos = p;
    Out.uv = vin.uv;
    return Out;
}

 
// ----------------------------------------------------------
// ピクセル
// ----------------------------------------------------------
Texture2D texture0 : register(t4);      // 4番のテクスチャスロットを使用（UNIDX_PS_SLOT_ALBEDO）
SamplerState sampler0 : register(s4);   // 4番のサンプラスロットを使用

// ピクセルシェーダー
float4 PS(PSInput In) : SV_Target0
{
    // テクスチャから色を取得
    float4 texColor = texture0.Sample(sampler0, In.uv);
 
    // テクスチャの色を出力
    return texColor;
}
