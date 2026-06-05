/*!
 * @brief G-Buffer出力用シェーダー（シャドウレシーバーなし）
 * キャラクターなど影を受けないモデル用。albedo.w = 0 で影無しを通知する。
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

Texture2D<float4>          g_texture     : register(t0);
Texture2D<float4>          g_specularMap : register(t2);
StructuredBuffer<float4x4> g_boneMatrix  : register(t3);
sampler                    g_sampler     : register(s0);

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
    SPSOut psOut;
    psOut.albedo      = g_texture.Sample(g_sampler, psIn.uv);
    psOut.albedo.w    = 0.0f;   // シャドウレシーバーフラグ（0=影を受けない）
    psOut.normal      = float4(psIn.normal * 0.5f + 0.5f, g_specularMap.Sample(g_sampler, psIn.uv).r);
    psOut.worldPos    = float4(psIn.worldPos, 1.0f);
    return psOut;
}
