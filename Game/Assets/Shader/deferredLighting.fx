/*!
 * @brief ディファードレンダリング用ライティングシェーダー
 * G-Bufferを読み込んで全光源のライティングを計算する
 */

// スプライト用定数バッファ (b0) — Sprite クラスが mvp/mulColor/screenParam を自動設定
cbuffer cb : register(b0)
{
    float4x4 mvp;
    float4   mulColor;
    float4   screenParam; // x=near, y=far, z=width, w=height（タイルインデックス計算用）
};

// ライト定数バッファ (b1) — SceneLight::Light に完全対応
cbuffer LightCb : register(b1)
{
    float3   dirDirection;   // ディレクションライトの方向
    float3   dirColor;       // ディレクションライトのカラー
    float3   lightPos;       // ライト位置（シャドウマップ用）
    float3   ptPosition;     // ポイントライトの位置
    float3   ptColor;        // ポイントライトのカラー
    float    ptRange;        // ポイントライトの影響範囲
    float3   spPosition;     // スポットライトの位置
    float3   spColor;        // スポットライトのカラー
    float    spRange;        // スポットライトの影響範囲
    float3   spDirection;    // スポットライトの照射方向
    float    spAngle;        // スポットライトの照射角度
    float3   eyePos;         // 視点の位置
    float    specPow;        // スペキュラの絞り
    float3   ambientLight;   // 環境光
    float3   groundColor;    // 地面色（半球ライト）
    float3   skyColor;       // 天球色（半球ライト）
    float3   groundNormal;   // 地面の法線（半球ライト）
    float4x4 mLVP;           // ライトビュープロジェクション行列
};

// G-Bufferテクスチャ
Texture2D<float4> albedoTexture   : register(t0);   // アルベド
Texture2D<float4> normalTexture   : register(t1);   // 法線（0〜1エンコード済）
Texture2D<float4> worldPosTexture : register(t2);   // ワールド座標
Texture2D<float2> shadowTexture   : register(t3);   // シャドウマップ（R32G32_FLOAT VSM）
sampler           Sampler         : register(s0);

// TBDR（タイルベースディファードレンダリング）用リソース
// t20, t21 は SpriteInitData::m_expandShaderResoruceView[0], [1] に対応
#define TILE_WIDTH  16
#define TILE_HEIGHT 16
#define MAX_TBDR_POINT_LIGHT 1000

// TBDRポイントライトデータ（CPU 側の TBDRPointLight に対応）
struct TBDRPointLight
{
    float4 position; // xyz=ワールド座標, w=pad0（C++ alignas(16) の 4byte パディング）
    float3 color;
    float  range;
};

StructuredBuffer<uint>          pointLightListInTile : register(t20); // タイル別ライトインデックスリスト
StructuredBuffer<TBDRPointLight> tbdrLightBuffer     : register(t21); // ライトデータ

struct VSInput
{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv  = In.uv;
    return psIn;
}

// Lambert拡散反射
float3 CalcLamberDiffuse(float3 lightDir, float3 lightColor, float3 normal)
{
    float t = dot(normal, lightDir) * -1.0f;
    t = t * 0.5f + 0.5f;
    t = max(0.0f, t);
    return lightColor * t;
}

// Phong鏡面反射（specPower: スペキュラマップから取得したピクセルごとの強度）
float3 CalcPhongSpecular(float3 lightDir, float3 lightColor, float3 worldPos, float3 normal, float specPower)
{
    float3 refVec = reflect(lightDir, normal);
    float3 toEye  = normalize(eyePos - worldPos);
    float t = max(0.0f, dot(refVec, toEye));
    t = pow(t, 5.0f);
    return lightColor * t * specPower;
}

// リムライト
float3 CalcLimLight(float3 lightDir, float3 lightColor, float3 normal, float3 worldPos)
{
    float3 toEye   = normalize(eyePos - worldPos);
    float power1   = 1.0f - abs(dot(lightDir, normal));
    float power2   = 1.0f - max(0.0f, dot(toEye, normal));
    float limPower = pow(power1 * power2, 1.0f);
    return limPower * lightColor * 2.0f;
}

// VSMシャドウ
float CalcShadow(float3 worldPos, float3 normal)
{
    float shadowAttn = 1.0f;
    float NdotL = dot(normal, -dirDirection);
    if (NdotL < 0.2f) return shadowAttn;

    float4 posInLVP   = mul(mLVP, float4(worldPos, 1.0f));
    float2 shadowUV   = posInLVP.xy / posInLVP.w;
    shadowUV         *= float2(0.5f, -0.5f);
    shadowUV         += 0.5f;
    float zInLVP      = posInLVP.z / posInLVP.w - 0.0005f;

    if (shadowUV.x > 0.0f && shadowUV.x < 1.0f &&
        shadowUV.y > 0.0f && shadowUV.y < 1.0f)
    {
        float texelSize  = 1.0f / 4096.0f;
        float totalAttn  = 0.0f;
        for (int x = -2; x <= 1; x++)
        {
            for (int y = -2; y <= 1; y++)
            {
                float2 smVal = shadowTexture.Sample(Sampler, shadowUV + float2(x, y) * texelSize);
                float  attn  = 1.0f;
                if (zInLVP > smVal.r)
                {
                    float md        = zInLVP - smVal.r;
                    float variance  = min(max(smVal.g - smVal.r * smVal.r, 0.0001f), 1.0f);
                    float litFactor = variance / (variance + (md * 1.5f) * (md * 1.5f));
                    if (litFactor < 0.9f) attn = 0.5f;
                }
                totalAttn += attn;
            }
        }
        shadowAttn = totalAttn / 16.0f;
    }
    return shadowAttn;
}

float4 PSMain(PSInput In) : SV_Target0
{
    // G-Bufferからデータを取得
    float4 albedo         = albedoTexture.Sample(Sampler, In.uv);
    float  shadowReceiver = albedo.w;   // 1=影を受ける、0=受けない（model_gbuffer_ns.fx）
    float4 worldPosSamp   = worldPosTexture.Sample(Sampler, In.uv);

    // worldPos.w = 0 はジオメトリなし（クリア値）→ メインRTのクリアカラーを透過させる
    // model_gbuffer.fx で psOut.worldPos = float4(worldPos, 1.0f) と明示的に書いているため信頼性が高い
    if (worldPosSamp.w < 0.5f) discard;

    float4 normalSamp = normalTexture.Sample(Sampler, In.uv);
    float3 normal     = normalize(normalSamp.xyz * 2.0f - 1.0f);
    float  specPower  = normalSamp.w;   // スペキュラマップ強度（G-Bufferのnormal.wに格納）
    float3 worldPos   = worldPosSamp.xyz;

    // ディレクションライト（拡散 + 鏡面 + リム）
    float3 dirLig = CalcLamberDiffuse(dirDirection, dirColor, normal)
                  + CalcPhongSpecular(dirDirection, dirColor, worldPos, normal, specPower)
                  + CalcLimLight(dirDirection, dirColor, normal, worldPos);

    // ポイントライト（拡散 + 鏡面、距離減衰あり）
    float3 ptDir     = normalize(worldPos - ptPosition);
    float3 ptLig     = CalcLamberDiffuse(ptDir, ptColor, normal)
                     + CalcPhongSpecular(ptDir, ptColor, worldPos, normal, specPower);
    float  ptDist    = length(worldPos - ptPosition);
    float  ptAffect  = pow(max(0.0f, 1.0f - ptDist / ptRange), ptRange);
    ptLig *= ptAffect;

    // スポットライト（拡散 + 鏡面、距離・角度減衰あり）
    float3 spDir         = normalize(worldPos - spPosition);
    float3 spLig         = CalcLamberDiffuse(spDir, spColor, normal)
                         + CalcPhongSpecular(spDir, spColor, worldPos, normal, specPower);
    float  spDist        = length(worldPos - spPosition);
    float  spDistAffect  = pow(max(0.0f, 1.0f - spDist / spRange), 3.0f);
    float  spAngleAffect = pow(max(0.0f, 1.0f - abs(acos(dot(spDir, spDirection))) / spAngle), 0.5f);
    spLig *= spDistAffect * spAngleAffect;

    // 半球ライト
    float  t       = (dot(normal, groundNormal) + 1.0f) * 0.5f;
    float3 hemiLig = lerp(groundColor, skyColor, t);

    // シャドウ（shadowReceiver=0のモデルは影を受けない）
    float shadowAttn = lerp(1.0f, CalcShadow(worldPos, normal), shadowReceiver);

    // ---- TBDR ポイントライト（タイル別カリング済み）----
    float3 tbdrLig = 0.0f;
    {
        uint2  viewportPos = (uint2)In.pos.xy;
        uint   numCellX    = ((uint)screenParam.z + TILE_WIDTH  - 1) / TILE_WIDTH;
        uint   tileIndex   = viewportPos.x / TILE_WIDTH + (viewportPos.y / TILE_HEIGHT) * numCellX;
        uint   lightStart  = tileIndex * MAX_TBDR_POINT_LIGHT;
        uint   lightEnd    = lightStart + MAX_TBDR_POINT_LIGHT;

        for (uint li = lightStart; li < lightEnd; li++)
        {
            uint ligNo = pointLightListInTile[li];
            if (ligNo == 0xffffffff) break;

            TBDRPointLight lig    = tbdrLightBuffer[ligNo];
            float3         ligDir = normalize(worldPos - lig.position.xyz);
            float          dist   = length(worldPos - lig.position.xyz);
            float          affect = max(0.0f, 1.0f - dist / lig.range);

            tbdrLig += CalcLamberDiffuse(ligDir, lig.color, normal) * affect;
            tbdrLig += CalcPhongSpecular(ligDir, lig.color, worldPos, normal, specPower) * affect;
        }
    }

    // ライト合算
    float3 totalLig = dirLig + ptLig + spLig + hemiLig + ambientLight + tbdrLig;

    float4 finalColor  = albedo;
    finalColor.xyz    *= totalLig * shadowAttn;

    return finalColor;
}
