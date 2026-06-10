/*!
 * 剣痕デカール用フォワードパスシェーダー。
 * デカールの寿命に合わせてフェードできるように、アルファは拡張CBで制御する。
 * b1レイアウト: float alpha, float3 pad (16バイト)
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
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

cbuffer DecalAlphaCb : register(b1)
{
    float  g_alpha;
    float3 pad;
};

Texture2D<float4> g_albedo  : register(t0);
SamplerState      g_sampler : register(s0);

SPSIn VSMain(SVSIn vsIn)
{
    SPSIn psIn;
    psIn.pos = mul(mProj, mul(mView, mul(mWorld, vsIn.pos)));
    psIn.uv  = vsIn.uv;
    return psIn;
}

SPSIn VSSkinMain(SVSIn vsIn) { return VSMain(vsIn); }

float4 PSMain(SPSIn psIn) : SV_Target0
{
    float4 color = g_albedo.Sample(g_sampler, psIn.uv);
    color.a *= g_alpha;
    if (color.a < 0.01f) discard;
    return color;
}

float4 PSNormalMain(SPSIn psIn) : SV_Target0
{
    return PSMain(psIn);
}
