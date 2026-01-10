// ----------------------------------------------------------
// 頂点
// ----------------------------------------------------------
// カメラ定数バッファ
cbuffer CBPerCamera : register(b8)
{
    float4x4 view;
    float4x4 projection;
    float3   cameraPosW;
    float    cameraNear;
    float3   cameraForwardW;
    float    cameraFar;
    float4   time; // (t, dt, 1/dt, frameCount)
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
    float2 uv  : TEXUV;
};

// 頂点シェーダーから出力するデータ＝ピクセルシェーダーに入力するデータ
struct PSInput
{
    float4 posH : SV_Position;  // 頂点の座標(射影座標系)
    float3 posW : TEXCOORD0;    // ワールド座標
    half3  nrmW : TEXCOORD1;    // ワールド法線
    half2  uv   : TEXCOORD2;    // UV座標
};


// 頂点シェーダー
PSInput VS(VSInput vin)
{
    PSInput Out;
    float4 p = float4(vin.pos.xyz, 1);
    p = mul(world, p);      // ワールド変換
    Out.posW = (float3)p;

    p = mul(view, p);       // ビュー変換
    p = mul(projection, p); // プロジェクション変換
    Out.posH = p;

    float3x3 world3x3 = (float3x3) world;
    Out.nrmW = mul(world3x3, vin.nrm);

    Out.uv = vin.uv;

    return Out;
}

 
// ----------------------------------------------------------
// ピクセル
// ----------------------------------------------------------
// マテリアル定数バッファ
cbuffer CBPerMaterial : register(b10)
{
    float4 baseColor;
};

// ライト
cbuffer CBLightPerFrame : register(b11)
{
    float4 ambientColor;
    float4 directionalColor;
    float3 directionW;
    uint pad;
};
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

// テクスチャとサンプラ。4番のテクスチャスロットとサンプラスロットを使用（UNIDX_PS_SLOT_ALBEDO）
Texture2D texture0 : register(t4);
SamplerState sampler0 : register(s4);


void EvaluatePointLight(in PointLight L, in float3 posW, in half3 nrmW,
    out half NdotL, out half atten)
{
    half3 Ldir = L.positionW - posW;
    half dist = length(Ldir);

    Ldir /= dist;
    atten = saturate(1 - dist * L.rangeInv);
    NdotL = dot(nrmW, Ldir);
}

void EvaluateSpotLight(in SpotLight L, in float3 posW, in half3 nrmW,
    out half NdotL, out half atten)
{
    half3 Ldir = L.positionW - posW;
    half dist = length(Ldir);
    Ldir /= dist;
    float spotCos = dot(-Ldir, L.directionW);
    if (spotCos <= L.outerCos)
    {
        atten = 0;
        NdotL = 0;
    }
    else
    {
        atten = saturate(1 - dist * L.rangeInv) * saturate((spotCos - L.outerCos) / (1 - L.outerCos));
        NdotL = dot(nrmW, Ldir);
    }
}


// ピクセルシェーダー
half4 PS(PSInput In) : SV_Target0
{
    // テクスチャから色を取得
    half4 albedo = texture0.Sample(sampler0, In.uv);

    // 明示的に法線を正規化（モデルスケール非均等だと崩れるため）
    half3 N = normalize(In.nrmW);

    // ライトループ
    half3 diffAccum;            // 拡散光
    diffAccum = ambientColor * ambientColor.a; // 環境光

    // ディレクショナルライト
    diffAccum += (directionalColor * directionalColor.a) * saturate(dot(N, -directionW));
    half NdotL, atten;
    uint i;

    // ポイントライト
    [loop]
    for (i = 0; i < pointLightCount; ++i)
    {
        EvaluatePointLight(pointLights[i], In.posW, N, NdotL, atten);
        half3 lc = pointLights[i].color * pointLights[i].color.a;
        diffAccum += lc * saturate(atten * NdotL);
    }

    // スポットライト
    [loop]
    for (i = 0; i < spotLightCount; ++i)
    {
        EvaluateSpotLight(spotLights[i], In.posW, N, NdotL, atten);
        half3 lc = spotLights[i].color * spotLights[i].color.a;
        diffAccum += lc * saturate(atten * NdotL);
    }

    // カラー合成
    half4 color = half4(diffAccum, 1) * albedo * (half4)baseColor;

    // 最終カラーを出力
    return color;
}
