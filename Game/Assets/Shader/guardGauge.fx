/*!
 * @brief	防御ゲージの残り時間表現
 *          テクスチャ自体がguard_frameと同じ縦向きの三日月形状のため、回転はさせず表示する。
 *          単純にuv.y（縦方向）だけで水平にクリップすると、カーブがきつい部分で
 *          境界線が弧の向きに対して斜めに横切ってしまい、減り方が形に合わなく見える。
 *          そのため中心線の形状（GAUGE_ARC_LUT、guard_gauge.pngのアルファ形状から算出）を元に、
 *          残量境界における弧の接線に垂直な直線でクリップし、上端から下端へ
 *          カーブに沿って欠けていくように表示する。
 *          QUAD_WIDTH/QUAD_HEIGHTはguardGaugeLayout.jsonのguardGaugeのwidth/heightと
 *          一致させること（傾きの計算にのみ使用、ズレても境界の上下位置自体はズレない）。
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;
    float4   mulColor; // x=残量比率(0.0~1.0), w=アルファ
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

// guard_gauge.pngのアルファ形状の中心線（x座標）をuv.yごとにサンプリングしたルックアップテーブル。
// x=uv.y（0=上端, 1=下端）, y=その高さでの中心線のuv.x
static const int LUT_COUNT = 17;
static const float2 GAUGE_ARC_LUT[LUT_COUNT] =
{
    float2(0.00000, 0.25865), float2(0.06255, 0.27692), float2(0.12510, 0.30577),
    float2(0.18765, 0.43654), float2(0.25020, 0.52981), float2(0.31274, 0.59904),
    float2(0.37529, 0.64519), float2(0.43784, 0.67308), float2(0.50039, 0.68173),
    float2(0.56216, 0.67115), float2(0.62471, 0.64519), float2(0.68726, 0.59904),
    float2(0.74980, 0.52981), float2(0.81235, 0.43558), float2(0.87490, 0.30481),
    float2(0.93745, 0.27692), float2(1.00000, 0.25769)
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
    float u          = In.uv.x;
    float v          = In.uv.y;
    float clip_ratio = mulColor.x; // 0.0(空) ~ 1.0(満タン)

    const float QUAD_WIDTH  = 24.0;
    const float QUAD_HEIGHT = 156.0;

    // uv.y=0が上端、uv.y=1が下端。残量分だけ下端側を残し、上端側から消していく
    float vb = 1.0 - clip_ratio;

    // 残量境界(vb)を挟むLUT区間を探し、その区間の中心線位置と傾きを求める
    int seg = 0;
    for (int i = 0; i < LUT_COUNT - 1; i++)
    {
        if (vb >= GAUGE_ARC_LUT[i].x)
        {
            seg = i;
        }
    }
    float2 p0 = GAUGE_ARC_LUT[seg];
    float2 p1 = GAUGE_ARC_LUT[seg + 1];
    float  t  = saturate((vb - p0.x) / max(p1.x - p0.x, 1e-6));
    float  cx = lerp(p0.y, p1.y, t);
    float  slope = ((p1.y - p0.y) * QUAD_WIDTH) / max((p1.x - p0.x) * QUAD_HEIGHT, 1e-6);

    // ワールド相当の単位に直し、中心線の接線に垂直な直線を境界にする
    float xWorld  = (u  - 0.5) * QUAD_WIDTH;
    float cxWorld = (cx - 0.5) * QUAD_WIDTH;
    float vbWorld = vb * QUAD_HEIGHT;
    float vWorld  = v  * QUAD_HEIGHT;

    float thresholdWorld = vbWorld - slope * (xWorld - cxWorld);
    if (vWorld < thresholdWorld)
    {
        discard;
    }

    // 白い縁取りはguard_frame側（ゲージの外側）に属するものなので、
    // 増減するゲージ本体のクリップには反映させない
    float4 tex = colorTexture.Sample(Sampler, In.uv);
    return float4(tex.rgb, tex.a * mulColor.w);
}
