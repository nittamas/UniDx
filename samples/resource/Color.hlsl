// ----------------------------------------------------------
// 頂点カラーを使うだけの単純なシェーダー
// 頂点に VertexPC を使う
// ----------------------------------------------------------

// ----------------------------------------------------------
// 頂点
// ----------------------------------------------------------
// カメラ定数バッファ
cbuffer CBPerCamera : register(b8)
{
    row_major float4x4 view;
    row_major float4x4 projection;
};

// オブジェクト定数バッファ
cbuffer CBPerObject : register(b9)
{
    row_major float4x4 world;
};

// 頂点シェーダーへ入力するデータ
struct VSInput
{
    float3 pos : POSITION;
    float4 color : COLOR0;
};


// 頂点シェーダーから出力するデータ
struct PSInput
{
    float4 pos : SV_POSITION; // 頂点の座標(射影座標系)
    float4 color : COLOR0;
};

// 頂点シェーダー
PSInput VS(VSInput vin)
{
    PSInput Out;
    float4 p = float4(vin.pos.xyz, 1);
    p = mul(p, world);
    p = mul(p, view);
    p = mul(p, projection);
    Out.pos = p;
    Out.color = vin.color;
    return Out;
}

 
// ----------------------------------------------------------
// ピクセル
// ----------------------------------------------------------

// ピクセルシェーダー
float4 PS(PSInput In) : SV_Target0
{
    // テクスチャの色を出力
    return In.color;
}
