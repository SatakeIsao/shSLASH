/*!
 * @brief 数字スプライトシート表示シェーダー（UVオフセット方式）
 *        numbers.DDSのアトラスから特定桁をUVオフセットで切り抜く
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;
    float4 mulColor;
};

// 数字選択用 UVパラメータ
cbuffer NumberCB : register(b1)
{
    float uvOffsetX; // (digit / 10)
    float uvScaleX;  // 1/10
    float uvOffsetY; // コンテンツY開始UV (余白をカット)
    float uvScaleY;  // コンテンツY幅UV
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
    psIn.uv.x = In.uv.x * uvScaleX + uvOffsetX;
    psIn.uv.y = In.uv.y * uvScaleY + uvOffsetY;
    return psIn;
}

float4 PSMain(PSInput In) : SV_Target0
{
    return colorTexture.Sample(Sampler, In.uv) * mulColor;
}
