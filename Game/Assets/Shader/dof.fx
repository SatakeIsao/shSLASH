cbuffer cb : register(b0)
{
    float4x4 mvp;
    float4   mulColor;
};

cbuffer DoFCb : register(b1)
{
    float4 eyePos;
    float4 viewForward;
    float4 playerWorldPos;
    float  nearBlur;
    float  farBlur;
    float  strength;
    float  playerRadius;
};

Texture2D<float4> bokeTexture     : register(t0);
Texture2D<float4> worldPosTexture : register(t1);

sampler Sampler : register(s0);

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

float4 PSMain(PSInput In) : SV_Target0
{
    float4 worldPos = worldPosTexture.Sample(Sampler, In.uv);

    float blendAlpha;

    if (worldPos.a < 0.5f)
    {
        /** ジオメトリなし（空・背景）：深度無限として扱い、最大ブラーを適用 */
        blendAlpha = strength;
    }
    else
    {
        /** プレイヤーの周囲にあるピクセルは処理をスキップ */
        float3 toPlayer = worldPos.xyz - playerWorldPos.xyz;
        if (dot(toPlayer, toPlayer) < playerRadius * playerRadius)
        {
            discard;
        }

        float3 toPoint = worldPos.xyz - eyePos.xyz;
        float  depth   = dot(toPoint, viewForward.xyz);

        if (depth < nearBlur)
        {
            discard;
        }

        float depthAlpha = saturate((depth - nearBlur) / max(farBlur - nearBlur, 0.001f));
        blendAlpha = depthAlpha * strength;
    }

    float4 boke = bokeTexture.Sample(Sampler, In.uv);
    boke.a = blendAlpha;
    return boke;
}
