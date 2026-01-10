// ----------------------------------------------------------
// 頂点
// ----------------------------------------------------------
cbuffer CBPerCamera : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
};

// 頂点シェーダーへ入力するデータ
struct VSInput
{
    float3 pos : POSITION;
    float2 uv : TEXUV;
    float4 color : COLOR0;
};


// 頂点シェーダーから出力するデータ
struct PSInput
{
    float4 pos : SV_POSITION; // 頂点の座標(射影座標系)
    float2 uv : TEXCOORD0; // UV座標
    float4 color : COLOR0;
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
    Out.color = vin.color;
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
    return texColor * In.color;
}
