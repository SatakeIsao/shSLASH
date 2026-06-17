cbuffer cb : register(b0)
{
    float4x4 mvp;
    float4   mulColor;
};

cbuffer MotionBlurCb : register(b1)
{
    float   strength;    // 0.0 - 1.0
    float   blurAmount;  // 1サンプルあたりのオフセット幅（0.0 - 0.1 程度）
    float2  center;      // ブラーの中心UV（通常 0.5, 0.5）
};

Texture2D<float4> mainTexture : register(t0);
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

static const int NUM_SAMPLES = 8;

float4 PSMain(PSInput In) : SV_Target0
{
    // 現ピクセルから画面中心へ向かう方向にサンプリング（収束型ラジアルブラー）
    float2 dir  = center - In.uv;
    float2 step = dir * (blurAmount / (float)NUM_SAMPLES);

    float4 color = (float4)0;
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        color += mainTexture.Sample(Sampler, In.uv + step * (float)i);
    }
    color /= (float)NUM_SAMPLES;
    color.a = strength;
    return color;
}
