/*!
 * @brief	HPバーの増減
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;
    float4   mulColor; // x=clip_ratio, y=left_w, z=right_w, w=unused
};

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

Texture2D<float4> colorTexture : register(t0);
sampler Sampler : register(s0);

PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv  = In.uv;
    return psIn;
}

float4 PSMain(PSInput In) : SV_Target0
{
    float u          = In.uv.x;
    float v          = In.uv.y;
    float clip_ratio = mulColor.x; // 0.0(空) ~ 1.0(満タン)
    float slant      = mulColor.y; // 斜めの傾き幅

    // 右端の斜めクリップ（テクスチャの右端斜めを再現）
    float right_clip = 1.0 - slant * v;
    if (u > right_clip)
    {
        discard;
    }

    // 左から削るクリップライン（斜めのままスライド）
    // clip_ratio=1.0で全表示、0.0で全非表示
    float left_clip = clip_ratio * (1.0 - slant) - slant * (v - 0.5);
    if (u > left_clip)
    {
        discard;
    }

    float hpRatio = mulColor.z;

float3 hpColor;
if (hpRatio > 0.5f)
{
    float t = (hpRatio - 0.5f) / 0.5f;
    hpColor = float3(1.0f - t, 1.0f, 0.0f); // 緑→黄
}
else
{
    float t = hpRatio / 0.5f;
    hpColor = float3(1.0f, t, 0.0f);         // 黄→赤
}

float4 tex = colorTexture.Sample(Sampler, In.uv);
return float4(hpColor * tex.rgb, tex.a * mulColor.w);
    //return colorTexture.Sample(Sampler, In.uv) * float4(1, 1, 1, mulColor.w);
}