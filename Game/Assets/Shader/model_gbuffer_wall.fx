/*!
 * @brief G-Buffer出力用シェーダー（壁・ステージ用 ディザリングあり）
 * modelWall.fx と同じディザリングロジックをG-Bufferパスに適用する。
 * ディザリングで clip() したピクセルはG-Bufferに書き込まれず、
 * ディファードライティングで discard されてフォワードパスの背景が透過する。
 */

// 4x4 ベイヤー行列（順序付きディザリング用）
static const int ditherPattern[4][4] = {
    { 0, 32,  8, 40},
    {48, 16, 56, 24},
    {12, 44,  4, 36},
    {60, 28, 52, 20},
};

static const float WALL_CLIP_RANGE = 125.0f;
static const float WALL_FADE_RANGE = 112.0f;

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

// SceneLight::Light と同じバイトレイアウト（eyePos と ditherEnabled を参照）
cbuffer LightCb : register(b1)
{
    float3   dirDirection;
    float3   dirColor;
    float3   lightPos;
    float3   ptPosition;
    float3   ptColor;
    float    ptRange;
    float3   spPosition;
    float3   spColor;
    float    spRange;
    float3   spDirection;
    float    spAngle;
    float3   eyePos;
    float    specPow;
    float3   ambientLight;
    float3   groundColor;
    float3   skyColor;
    float3   groundNormal;
    float4x4 mLVP;
    int      ditherEnabled;
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
    // ディザリング（modelWall.fx と同じロジック）
    if (ditherEnabled != 0)
    {
        int dx = (int)fmod(psIn.pos.x, 4.0f);
        int dy = (int)fmod(psIn.pos.y, 4.0f);
        int dither = ditherPattern[dy][dx];

        float distToEye    = length(psIn.worldPos - eyePos);
        float eyeToClip    = max(0.0f, distToEye - WALL_CLIP_RANGE);
        float clipRate     = 1.0f - min(1.0f, eyeToClip / WALL_FADE_RANGE);

        // 床・天井はディザリングしない
        if (abs(psIn.normal.y) > 0.8f) clipRate = 0.0f;
        // カメラが真上から見ているときは曲面もディザリングしない
        if (abs(psIn.normal.y) > 0.3f && eyePos.y > psIn.worldPos.y + 50.0f) clipRate = 0.0f;

        clip(dither - 64 * clipRate);
    }

    SPSOut psOut;
    psOut.albedo   = g_texture.Sample(g_sampler, psIn.uv);
    psOut.albedo.w = 1.0f;  // シャドウレシーバーフラグ（1=影を受ける）
    psOut.normal   = float4(psIn.normal * 0.5f + 0.5f, g_specularMap.Sample(g_sampler, psIn.uv).r);
    psOut.worldPos = float4(psIn.worldPos, 1.0f);
    return psOut;
}
