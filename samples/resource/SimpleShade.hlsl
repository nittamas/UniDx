// ----------------------------------------------------------
// テクスチャ１枚、環境光、ディレクショナルライト、
// マテリアルカラーだけの単純な陰影シェーダー
// ----------------------------------------------------------

// ----------------------------------------------------------
// 頂点シェーダ
// ----------------------------------------------------------
// カメラ定数バッファ
cbuffer CBPerCamera : register(b8)
{
    float4x4 view;
    float4x4 projection;
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
    float3 nrm : NORMAL;
};

// 頂点シェーダーから出力するデータ
struct PSInput
{
    float4 posH : SV_Position;  // 頂点の座標(射影座標系)
    float3 nrmW : TEXCOORD0;    // 法線
};

// 頂点シェーダー
PSInput VS(VSInput vin)
{
    PSInput Out;

    float4 p = float4(vin.pos.xyz, 1);
    p = mul(world, p);      // ワールド変換
    p = mul(view, p);       // ビュー変換
    p = mul(projection, p); // プロジェクション変換
    Out.posH = p;

    float3x3 world3x3 = (float3x3) world;
    Out.nrmW = mul(world3x3, vin.nrm);
    return Out;
}


// ----------------------------------------------------------
// ピクセルシェーダ
// ----------------------------------------------------------
// マテリアル定数バッファ
cbuffer CBPerMaterial : register(b10)
{
    float4 baseColor;
};

// 共通ライト
cbuffer CBLightPerFrame : register(b11)
{
    float4 ambientColor;
    float4 directionalColor;
    float3 directionW;
    uint pad;
};

// ピクセルシェーダー
float4 PS(PSInput In) : SV_Target0
{
    // 明示的に法線を正規化（モデルスケール非均等だと崩れるため）
    float3 N = normalize(In.nrmW);

    // ライトループ
    float4 diffAccum = ambientColor; // 環境光
    diffAccum += directionalColor * saturate(dot(N, directionW)); // ディレクショナルライト

    // カラー合成
    float4 color = diffAccum * baseColor;

    // テクスチャの色を出力
    return color;
}
