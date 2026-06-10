/*!
 * 剣痕デカール用フォワードパスのダミーシェーダー。
 * デカールはすでにG-Bufferへ書き込まれているため、フォワード描画はすべて破棄する。
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
};

cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

SPSIn VSMain(SVSIn vsIn)
{
    SPSIn psIn;
    psIn.pos = mul(mProj, mul(mView, mul(mWorld, vsIn.pos)));
    return psIn;
}

SPSIn VSSkinMain(SVSIn vsIn) { return VSMain(vsIn); }

float4 PSMain(SPSIn psIn) : SV_Target0
{
    discard;
    return float4(0, 0, 0, 0);
}

float4 PSNormalMain(SPSIn psIn) : SV_Target0
{
    discard;
    return float4(0, 0, 0, 0);
}
