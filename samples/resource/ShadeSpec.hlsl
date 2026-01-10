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

// 行列定数バッファ
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

// 頂点シェーダーから出力するデータ＝ピクセルシェーダーに入力するデータ
struct PSInput
{
    float4 posH : SV_Position;  // 頂点の座標(射影座標系)
    float3 posW : TEXCOORD0;    // ワールド座標
    half3  nrmW : TEXCOORD1;    // ワールド法線
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
    uint   pad;
};
struct PointLight
{
    float4 color;
    float3 positionW;
    float  rangeInv;
};
struct SpotLight
{
    float4 color;
    float3 positionW;
    float  rangeInv;
    float3 directionW;
    float  outerCos;
};
cbuffer CBLightPerObject : register(b12)
{
    PointLight pointLights[8]; // 最大8個
    SpotLight spotLights[8];   // 最大8個
    uint pointLightCount;
    uint spotLightCount;
    float2 _pad; // 16byte 境界合わせ
};


void EvaluatePointLight(in PointLight L, in float3 posW, in half3 nrmW,
    out half NdotL, out half NdotH, out half atten)
{
    half3 Ldir = L.positionW - posW;
    half dist = length(Ldir);

    Ldir /= dist;
    atten = saturate(1 - dist * L.rangeInv);
    NdotL = dot(nrmW, Ldir);
    
    half3 V = normalize(cameraPosW - posW);
    half3 H = normalize(Ldir + V);
    NdotH = dot(nrmW, H);

}

void EvaluateSpotLight(in SpotLight L, in float3 posW, in half3 nrmW,
    out half NdotL, out half NdotH, out half atten)
{
    half3 Ldir = L.positionW - posW;
    half dist = length(Ldir);
    Ldir /= dist;
    float spotCos = dot(-Ldir, L.directionW);
    if (spotCos <= L.outerCos)
    {
        atten = 0;
        NdotL = 0;
        NdotH = 0;
    }
    else
    {
        atten = saturate(1 - dist * L.rangeInv) * saturate((spotCos - L.outerCos) / (1 - L.outerCos));
        NdotL = dot(nrmW, Ldir);

        half3 V = normalize(cameraPosW - posW);
        half3 H = normalize(Ldir + V);
        NdotH = dot(nrmW, H);
    }
}


// ピクセルシェーダー
half4 PS(PSInput In) : SV_Target0
{
    // 明示的に法線を正規化（モデルスケール非均等だと崩れるため）
    half3 N = normalize(In.nrmW);

    // ライトループ
    const half shininess = 100;
    const half3 specularColor = half3(0.2, 0.2, 0.2);

    half3 diffAccum;          // 拡散光
    half3 spec;               // 鏡面反射光
    diffAccum = ambientColor * ambientColor.a; // 環境光
    
    // ディレクショナルライト
    diffAccum += (directionalColor * directionalColor.a) * saturate(dot(N, -directionW));

    float3 L = normalize(-directionW);
    float3 V = normalize(cameraPosW - In.posW);
    half3 H = normalize(L + V);
    spec = directionalColor * pow(saturate(dot(N, H)), shininess);

    half NdotL, NdotH, atten;
    uint i;

    // ポイントライト
    [loop]
    for (i = 0; i < pointLightCount; ++i)
    {
        EvaluatePointLight(pointLights[i], In.posW, N, NdotL, NdotH, atten);
        half3 lc = pointLights[i].color * pointLights[i].color.a;
        diffAccum += lc * saturate(atten * NdotL);
        spec += lc * pow(saturate(NdotH), shininess);
    }

    // スポットライト
    [loop]
    for (i = 0; i < spotLightCount; ++i)
    {
        EvaluateSpotLight(spotLights[i], In.posW, N, NdotL, NdotH, atten);
        half3 lc = spotLights[i].color * spotLights[i].color.a;
        diffAccum += lc * saturate(atten * NdotL);
        spec += lc * pow(saturate(NdotH), shininess);
    }

    // カラー合成
    half4 color = half4(diffAccum, 1) + half4(spec * specularColor, 0);

    // 最終カラーを出力
    return color * (half4) baseColor;
}
