/*!
 * @brief 敵モデル用Gバッファーシェーダー（Bayerディザフェード対応）
 */

struct SSkinVSIn
{
    int4   Indices : BLENDINDICES0;
    float4 Weights : BLENDWEIGHT0;
};

struct SVSIn
{
    float4 pos    : POSITION;
    float3 normal : NORMAL;
    float2 uv     : TEXCOORD0;
    SSkinVSIn skinVert;
};

struct SPSIn
{
    float4 pos      : SV_POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

struct SPSOut
{
    float4 albedo   : SV_Target0;
    float4 normal   : SV_Target1;
    float4 worldPos : SV_Target2;
};

cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

cbuffer FadeCb : register(b1)
{
    float fadeRatio;
    float3 pad;
};

Texture2D<float4>          g_texture     : register(t0);
Texture2D<float4>          g_specularMap : register(t2);
StructuredBuffer<float4x4> g_boneMatrix  : register(t3);
sampler                    g_sampler     : register(s0);

// 4x4 Bayer順序付きディザマトリクス（0〜15 / 16）
static const float Bayer4x4[4][4] =
{
    {  0.0/16.0,  8.0/16.0,  2.0/16.0, 10.0/16.0 },
    { 12.0/16.0,  4.0/16.0, 14.0/16.0,  6.0/16.0 },
    {  3.0/16.0, 11.0/16.0,  1.0/16.0,  9.0/16.0 },
    { 15.0/16.0,  7.0/16.0, 13.0/16.0,  5.0/16.0 }
};

float4x4 CalcSkinMatrix(SSkinVSIn skinVert)
{
    float4x4 skinning = 0;
    float w = 0.0f;
    [unroll]
    for (int i = 0; i < 3; i++)
    {
        skinning += g_boneMatrix[skinVert.Indices[i]] * skinVert.Weights[i];
        w += skinVert.Weights[i];
    }
    skinning += g_boneMatrix[skinVert.Indices[3]] * (1.0f - w);
    return skinning;
}

SPSIn VSMainCore(SVSIn vsIn, uniform bool hasSkin)
{
    SPSIn psIn;
    float4x4 worldMatrix = hasSkin ? CalcSkinMatrix(vsIn.skinVert) : mWorld;

    float4 worldPos4 = mul(worldMatrix, vsIn.pos);
    psIn.worldPos = worldPos4.xyz;
    psIn.pos      = mul(mView, worldPos4);
    psIn.pos      = mul(mProj, psIn.pos);
    psIn.normal   = normalize(mul((float3x3)worldMatrix, vsIn.normal));
    psIn.uv       = vsIn.uv;
    return psIn;
}

SPSIn VSMain(SVSIn vsIn)     { return VSMainCore(vsIn, false); }
SPSIn VSSkinMain(SVSIn vsIn) { return VSMainCore(vsIn, true); }

SPSOut PSMain(SPSIn psIn)
{
    // Bayerディザで透明度を表現（fadeRatio: 0=完全透明, 1=完全不透明）
    int px = (int)psIn.pos.x & 3;
    int py = (int)psIn.pos.y & 3;
    if (Bayer4x4[py][px] >= fadeRatio) discard;

    SPSOut psOut;
    psOut.albedo      = g_texture.Sample(g_sampler, psIn.uv);
    psOut.albedo.w    = 0.0f;   // 影を受けない（自己シャドウ無効）
    psOut.normal      = float4(psIn.normal * 0.5f + 0.5f, g_specularMap.Sample(g_sampler, psIn.uv).r);
    psOut.worldPos    = float4(psIn.worldPos, 1.0f);
    return psOut;
}
