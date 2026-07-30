/*!
 * @brief	防御ゲージの枠（guard_frame）用シェーダー
 *          フレームは常にフルサイズで表示され、増減するゲージ本体とは別レイヤーのため、
 *          縁の白ハイライトはここに実装する（ゲージの残量には一切影響されない）。
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;
    float4   mulColor; // w=アルファ（x,y,zは未使用）
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
    float4 tex = colorTexture.Sample(Sampler, In.uv);

    // バー部分（アイコン部分より下）の左右端・下端を白く縁取る
    const float BAR_REGION_START = 0.15;  // uv.y。これより上はアイコンなので縁取りしない
    const float EDGE_WIDTH_U     = 0.22;  // 左右端の縁の太さ（uv.x基準）
    const float EDGE_FEATHER_U   = 0.06;

    // uv.xとuv.yはframeの実サイズ（guardGaugeLayout.jsonのwidth/height）に対して縮尺が全く違うため、
    // 同じ太さの値をそのままuv.yに使うと下端だけ極端に太くなってしまう。
    // frameの縦横比(width/height)で換算し、見た目の太さを左右端と揃える
    const float FRAME_WIDTH      = 30.0;
    const float FRAME_HEIGHT     = 156.0;
    const float EDGE_WIDTH_V     = EDGE_WIDTH_U   * (FRAME_WIDTH / FRAME_HEIGHT);
    const float EDGE_FEATHER_V   = EDGE_FEATHER_U * (FRAME_WIDTH / FRAME_HEIGHT);

    if (In.uv.y > BAR_REGION_START)
    {
        float u = In.uv.x;
        float distFromSide   = min(u, 1.0 - u);  // 0=左右端、大きいほど中心
        float distFromBottom = 1.0 - In.uv.y;    // 0=下端、大きいほど上

        float edgeSide   = 1.0 - smoothstep(EDGE_WIDTH_U - EDGE_FEATHER_U, EDGE_WIDTH_U, distFromSide);
        float edgeBottom = 1.0 - smoothstep(EDGE_WIDTH_V - EDGE_FEATHER_V, EDGE_WIDTH_V, distFromBottom);
        float edge = max(edgeSide, edgeBottom);

        tex.rgb = lerp(tex.rgb, float3(1.0, 1.0, 1.0), edge);
    }

    return float4(tex.rgb, tex.a * mulColor.w);
}
