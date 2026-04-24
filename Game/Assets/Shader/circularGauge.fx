/*!
 * @brief 円形ゲージ描画シェーダー
 *
 * UV座標から角度を計算し、数学的に円弧を描画する。
 * テクスチャは使用しない。
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;
    float4   mulColor;
};

// ゲージパラメータ (b1)
cbuffer GaugeCB : register(b1)
{
    float4 fillColor;    // 1周目の塗り色
    float4 fillColor2;   // 2周目の塗り色
    float4 emptyColor;   // 空エリアの色
    float  startAngle;   // 開始角度 (ラジアン、0=上、PI/2=右、PI=下、3PI/2=左)
    float  fillAmount;   // 塗り量 (0.0〜2.0、1.0=1周、2.0=2周)
    float  arcSpan;      // 円弧の広さ (2PI=全円、PI=半円)
    float  innerRadius;  // 内側の半径比率 (0=塗りつぶし円、0.5=リング)
    float  outerRadius;  // 外側の半径比率 (1.0=スプライト全体、0.8=少し小さめ)
    float3 _pad;         // パディング
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

static const float PI  = 3.14159265f;
static const float PI2 = 6.28318530f;

PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv  = In.uv;
    return psIn;
}

float4 PSMain(PSInput In) : SV_Target0
{
    // UV中心を (0,0) に移す
    float2 dir = In.uv - float2(0.5f, 0.5f);

    // 外径・内径チェック（r は 0〜1、外縁で 1.0）
    float r = length(dir) * 2.0f;
    if (r > outerRadius)
        discard;
    if (r < innerRadius)
        discard;

    // 角度計算: atan2(x, -y) → 0=上、時計回りで π/2=右、π=下、3π/2=左
    float rawAngle = atan2(dir.x, -dir.y);
    if (rawAngle < 0.0f)
        rawAngle += PI2;

    // 開始角度からの相対角度 (0〜2PI)
    float localAngle = fmod(rawAngle - startAngle + PI2, PI2);

    // arcSpan 外は空エリア
    if (localAngle > arcSpan)
        return emptyColor * mulColor;

    float clampedFill = clamp(fillAmount, 0.0f, 2.0f);
    float fill1End = min(clampedFill, 1.0f) * arcSpan;          // 1周目の塗り終端角度
    float fill2End = max(clampedFill - 1.0f, 0.0f) * arcSpan;   // 2周目の塗り終端角度

    if (fill2End > 0.0f && localAngle < fill2End)
    {
        // 2周目の塗りエリア
        return fillColor2 * mulColor;
    }
    else if (localAngle < fill1End || fill1End >= arcSpan)
    {
        // 1周目の塗りエリア（fill1End >= arcSpan は1周完了を意味する）
        return fillColor * mulColor;
    }
    else
    {
        return emptyColor * mulColor;
    }
}
