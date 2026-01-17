// ----------------------------------------------------------
// 環境光、ディレクショナルライト、
// マテリアルカラーだけの単純な陰影シェーダー
// 頂点に VertexPN を使う
// ----------------------------------------------------------

// ----------------------------------------------------------
// 頂点
// ----------------------------------------------------------
// カメラ定数バッファ
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

// オブジェクト定数バッファ
struct BoneMat3x4
{
    float4 r0;
    float4 r1;
    float4 r2;
};
cbuffer CBSkinPerObject : register(b9)
{
    row_major float4x4 world;
    BoneMat3x4 bones[256];
};

// 頂点シェーダーへ入力するデータ
struct VSInput
{
    float3 pos : POSITION;
    float3 nrm : NORMAL;
    float4 tangent : TANGENT; // w = handedness
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    uint4 joints : BLENDINDICES; // R8G8B8A8_UINT → uint4
    float4 weights : BLENDWEIGHT; // R8G8B8A8_UNORM → float4(0..1)
};

// 頂点シェーダーから出力するデータ＝ピクセルシェーダーに入力するデータ
struct PSInput
{
    float4 posH : SV_Position;  // 頂点の座標(射影座標系)
    float3 posW : TEXCOORD0;    // ワールド座標
    half3  nrmW : TEXCOORD1;    // ワールド法線
    half2  uv   : TEXCOORD2;    // UV座標
    half4 color : TEXCOORD3;
};
BoneMat3x4 BoneBlend4(
    BoneMat3x4 b0, float w0,
    BoneMat3x4 b1, float w1,
    BoneMat3x4 b2, float w2,
    BoneMat3x4 b3, float w3)
{
    BoneMat3x4 o;
    o.r0 = b0.r0 * w0 + b1.r0 * w1 + b2.r0 * w2 + b3.r0 * w3;
    o.r1 = b0.r1 * w0 + b1.r1 * w1 + b2.r1 * w2 + b3.r1 * w3;
    o.r2 = b0.r2 * w0 + b1.r2 * w1 + b2.r2 * w2 + b3.r2 * w3;
    return o;
}
float3 BoneTransformPoint(float3 p, BoneMat3x4 m)
{
    // p' = R*p + t （Rは3x3、tは平行移動）
    float4 v = float4(p, 1.0);
    return float3(
        dot(v, m.r0),
        dot(v, m.r1),
        dot(v, m.r2)
    );
}
float3 BoneTransformVector(float3 v, BoneMat3x4 m)
{
    // 法線・接線など：平行移動なし
    float4 v4 = float4(v, 0.0);
    return float3(
        dot(v4, m.r0),
        dot(v4, m.r1),
        dot(v4, m.r2)
    );
}

// 頂点シェーダー
PSInput VS(VSInput vin)
{
    PSInput Out;

    uint4 j = vin.joints;
    float4 w = vin.weights;

    BoneMat3x4 bm;

    // ウェイト正規化
    float ws = w.x + w.y + w.z + w.w;
    if (ws > 0)
    {
        w /= ws;
        bm = BoneBlend4(bones[j.x], w.x, bones[j.y], w.y, bones[j.z], w.z, bones[j.w], w.w);
    }
    else
    {
        bm.r0 = float4(1, 0, 0, 0);
        bm.r1 = float4(0, 1, 0, 0);
        bm.r2 = float4(0, 0, 1, 0);
    }

    // ブレンド位置、法線、接線xyz
    float3 skPos = BoneTransformPoint(vin.pos, bm);
    float3 skNrm = BoneTransformVector(vin.nrm, bm);
    float3 skTan = BoneTransformVector(vin.tangent.xyz, bm);
        
    float4 p = float4(skPos.xyz, 1);
    p = mul(p, world); // ワールド変換
    Out.posW = (float3) p;

    p = mul(p, view); // ビュー変換
    p = mul(p, projection); // プロジェクション変換
    Out.posH = p;

    float3x3 world3x3 = (float3x3) world;

    float3 nW = mul(skNrm, world3x3);
    Out.nrmW = normalize((half3)nW);

    Out.uv = vin.uv0;
    float wm = max(max(w.x, w.y), max(w.z, w.w));
    Out.color = half4(wm, wm, wm, 1);
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

// テクスチャとサンプラ。4番のテクスチャスロットとサンプラスロットを使用（UNIDX_PS_SLOT_ALBEDO）
Texture2D texture0 : register(t4);
SamplerState sampler0 : register(s4);

// ポイントライトの影響を計算
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

// スポットライトの影響を計算
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
    const half alphaCutoff = 0.05;

    // テクスチャから色を取得
    half4 albedo = texture0.Sample(sampler0, In.uv);

    // テクスチャとbaseColorのアルファでクリップ
    clip(albedo.a * baseColor.a - alphaCutoff);

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
        spec += lc * pow(saturate(NdotH), shininess); // Blinn-Phong
    }

    // スポットライト
    [loop]
    for (i = 0; i < spotLightCount; ++i)
    {
        EvaluateSpotLight(spotLights[i], In.posW, N, NdotL, NdotH, atten);
        half3 lc = spotLights[i].color * spotLights[i].color.a;
        diffAccum += lc * saturate(atten * NdotL);
        spec += lc * pow(saturate(NdotH), shininess); // Blinn-Phong
    }

    // カラー合成
    half4 color = half4(diffAccum, 1) * albedo + half4(spec * specularColor, 0);
//    color = In.color;

    // 最終カラーを出力
    return color * (half4) baseColor;
}
