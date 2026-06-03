/*!
 * @brief	壁用モデルシェーダー。
 *          ディザリングで常に半透明表示する。
 */

//定数
static const int NUM_DIRECTIONAL_LIGHT = 4; //ディレクションライトの数
static const int MAX_POINT_LIGHT = 32;      //ポイントライトの最大数
static const int MAX_SPOT_LIGHT = 32;       //スポットライトの最大数

// 4x4 ベイヤー行列（順序付きディザリング用）
static const int ditherPattern[4][4] = {
    { 0, 32,  8, 40},
    {48, 16, 56, 24},
    {12, 44,  4, 36},
    {60, 28, 52, 20},
};

// カメラがこの距離より近づくとフェード開始
static const float WALL_CLIP_RANGE = 125.0f;
static const float WALL_FADE_RANGE = 112.0f;

// アリーナの内側半径（ステージのサイズに合わせて調整）
static const float ARENA_WALL_RADIUS = 250.0f;

////////////////////////////////////////////////
// 構造体
////////////////////////////////////////////////
//スキニング用の頂点データをひとまとめ。
struct SSkinVSIn
{
	int4   Indices  : BLENDINDICES0;
    float4 Weights  : BLENDWEIGHT0;
};
//頂点シェーダーへの入力。
struct SVSIn
{
    float4 pos			: POSITION;		//モデルの頂点座標。
    float3 normal		: NORMAL;		//法線。

    float3 tangent		: TANGENT;		//接ベクトル。
    float4 biNormal		: BINORMAL;		//従ベクトル。

    float2 uv			: TEXCOORD0;	//UV座標。

	SSkinVSIn skinVert;					//スキン用のデータ。
};
//ピクセルシェーダーへの入力。
struct SPSIn
{
	float4 pos 			: SV_POSITION;	//スクリーン空間でのピクセルの座標。
    float3 normal		: NORMAL;		//法線。

    float3 tangent		: TANGENT;		//接ベクトル。
    float3 biNormal		: BINORMAL;		//従ベクトル。

    float2 uv			: TEXCOORD0;	//UV座標
    float3 worldPos		: TEXCOORD1;	//ワールド空間座標。
    float3 normalInView : TEXCOORD2;    //カメラ空間の法線。

    float4 posInLVP     : TEXCOORD3;    //ライトビュースクリーン空間でのピクセルの座標
};

struct SPSOut
{
    float4 color : SV_Target0;
};

////////////////////////////////////////////////
// 定数バッファ
////////////////////////////////////////////////
// モデル用の定数バッファ
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

cbuffer LightCb : register(b1)
{
    //ディレクションライト用のデータ
    float3 dirDirection;    //ライトの方向
    float3 dirColor;        //ライトのカラー
    float3 lightPos;

    //ポイントライト用のデータ
    float3 ptPosition;      //ポイントライトの位置
    float3 ptColor;         //ポイントライトのカラー
    float ptRange;          //ポイントライトの影響範囲


    //スポットライト用のデータ
    float3 spPosition;      //スポットライトの位置
    float3 spColor;         //スポットライトのカラー
    float  spRange;         //スポットライトの射出範囲
    float3 spDirection;     //スポットライトの射出方向
    float  spAngle;         //スポットライトの射出角度


    float3 eyePos;          //視点の位置
    float specPow;          //スペキュラの絞り
    float3 ambientLight;    //環境光

    //半球ライトのデータ
    float3 groundColor;     //折り返しのライト
    float3 skyColor;        //天球ライト
    float3 groundNormal;    //地面の法線

    float4x4 mLVP;

    int ditherEnabled;
}

////////////////////////////////////////////////
// シェーダーリソース
////////////////////////////////////////////////
Texture2D<float4> g_texture : register(t0);             //モデルテクスチャ
Texture2D<float4> g_normalMap : register(t1);           //法線マップ
Texture2D<float4> g_speclarMap : register(t2);          //スペキュラマップ
Texture2D<float4> g_shadowMap : register(t10);           //シャドウマップ

StructuredBuffer<float4x4> g_boneMatrix : register(t3);	//ボーン行列。

sampler g_sampler : register(s0);	                    //サンプラステート。

SamplerComparisonState g_shadowSampler : register(s1);  //シャドウマップ用の比較サンプラー



////////////////////////////////////////////////
// 関数宣言。
////////////////////////////////////////////////

// Lamber拡散反射光の計算
float3 CalcLamberDiffuse(float3 lightDirection, float3 lightColor, float3 normal);
float3 CalcPhongSpecular(float3 lightDirection, float3 lightColor, float3 worldPos, float3 normal,float2 uv);
float3 CalcLigFromDirectionLight(SPSIn psIn);
float3 CalcLigFromPointLight(SPSIn psIn);
float3 CalcLigFromSpotLight(SPSIn psIn);
float3 CalcLigFromLimLight(float3 lightDirection, float3 lightColor, float3 normal, float3 normalInView);
float3 CalcLigFromHemiLight(SPSIn psIn);
float3 CalcNormal(float3 normal, float3 tangent, float3 biNormal, float2 uv);
float CalcShadowMap(SPSIn psIn);

/// <summary>
//スキン行列を計算する。
/// </summary>
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

/// <summary>
/// 頂点シェーダーのコア関数。
/// </summary>
SPSIn VSMainCore(SVSIn vsIn, uniform bool hasSkin)
{
    SPSIn psIn;
    float4x4 worldMatrix;
    if (!hasSkin){
        worldMatrix = mWorld;
    }
    else{
        worldMatrix = CalcSkinMatrix(vsIn.skinVert);
    }
    psIn.pos = mul(worldMatrix, vsIn.pos); //モデルの頂点をワールド座標系に変換
    psIn.worldPos = psIn.pos;
    psIn.pos = mul(mView, psIn.pos); //ワールド座標系からカメラ座標系に変換
    psIn.pos = mul(mProj, psIn.pos); //カメラ座標系からスクリーン座標系に変換

    psIn.normal = normalize(mul(worldMatrix, vsIn.normal)); //法線を回転させる。

    psIn.tangent = normalize(mul(worldMatrix, vsIn.tangent)); //接ベクトルをワールド空間に変換する
    psIn.biNormal = normalize(mul(worldMatrix, vsIn.biNormal)); //従ベクトルをワールド空間に変換する

    psIn.uv = vsIn.uv;

    psIn.normalInView = mul(mView, psIn.normal); //カメラ空間の法線を求める

    float4 worldPos = mul(mWorld, vsIn.pos);
    psIn.posInLVP = mul(mLVP, worldPos);    //ライトビュースクリーン空間の座標を計算する

    return psIn;
}

/// <summary>
/// スキンなしメッシュ用の頂点シェーダーのエントリー関数。
/// </summary>
SPSIn VSMain(SVSIn vsIn)
{
    return VSMainCore(vsIn, false);
}

/// <summary>
/// スキンありメッシュの頂点シェーダーのエントリー関数。
/// </summary>
SPSIn VSSkinMain(SVSIn vsIn)
{
    return VSMainCore(vsIn, true);
}


/// <summary>
/// ピクセルシェーダーのエントリー関数。
/// </summary>
SPSOut PSMainCore(SPSIn psIn, int isShadowReceiver) : SV_Target0
{
    SPSOut psOut;

    // ディザリング：壁面のみカメラ近接時に透過（床・天井は除外）
    if (ditherEnabled != 0)
    {
        int dx = (int)fmod(psIn.pos.x, 4.0f);
        int dy = (int)fmod(psIn.pos.y, 4.0f);
        int dither = ditherPattern[dy][dx];

        float distToEye = length(psIn.worldPos - eyePos);
        float eyeToClipRange = max(0.0f, distToEye - WALL_CLIP_RANGE);
        float clipRate = 1.0f - min(1.0f, eyeToClipRange / WALL_FADE_RANGE);

        // 床・天井（abs(normal.y) > 0.8）はディザリングしない
        if (abs(psIn.normal.y) > 0.8f) clipRate = 0.0f;
        // 曲面（接合部）はカメラがその面より50以上高いときはディザリングしない
        // （真上からだと暗い外側が透けるため。カメラが床付近なら許可）
        if (abs(psIn.normal.y) > 0.3f && eyePos.y > psIn.worldPos.y + 50.0f) clipRate = 0.0f;

        clip(dither - 64 * clipRate);
    }

    // ディレクションライトによるライティングを計算する
    float3 directionLig = CalcLigFromDirectionLight(psIn);

    // ポイントライトによるライティングを計算する
    float3 pointLig = CalcLigFromPointLight(psIn);

    // スポットライトによるライティングを計算する
    float spotLig = CalcLigFromSpotLight(psIn);

    // リムライトによるライティングを計算する
    float limLig = CalcLigFromLimLight(dirDirection, dirColor, psIn.normal, psIn.normalInView);

    // 半球ライトによるライティングを計算する
    float hemiLig = CalcLigFromHemiLight(psIn);

    //////////////////////////////////////////////////
    // 半球ライトを計算する
    // サーフェイスの法線と地面の法線との内積を計算する
    float t = dot(psIn.normal, groundNormal);

    // 内積の結果を0～1の範囲に変換する
    t = (t + 1.0f) / 2.0f;

    // 地面色と天球色を補完率tで線形補完する
    float3 hemiLight = lerp(groundColor, skyColor, t);
    /////////////////////////////////////////////////////

    //ディフューズマップをサンプリング
    float4 diffuseMap = g_texture.Sample(g_sampler, psIn.uv);

    // 各種ライトの反射光を足し算して最終的な反射光を求める
    float3 finalLig = directionLig + hemiLig;

    float4 finalColor = diffuseMap;
    // テクスチャカラーに求めた光を乗算して最終出力カラーを求める
    finalColor.xyz *= finalLig;

    float shadowAttn = 1.0f;
    if (isShadowReceiver == 1)
    {
        shadowAttn = CalcShadowMap(psIn);
        // 床は影を薄くする（強い影で黒く見えるのを防ぐ）
        if (abs(psIn.normal.y) > 0.8f)
            shadowAttn = max(shadowAttn, 0.7f);
    }

    finalColor.xyz *= shadowAttn;

    psOut.color = finalColor;

    return psOut;

}



SPSOut PSShadowReceverMain(SPSIn psIn) : SV_Target0
{
    return PSMainCore(psIn, 1);
}

SPSOut PSNormalMain(SPSIn psIn) : SV_Target0
{
    return PSMainCore(psIn, 0);
}

////////////////////////////////////////////////
// 関数定義。
////////////////////////////////////////////////

// Lamber拡散反射光の計算
float3 CalcLamberDiffuse(float3 lightDirection, float3 lightColor, float3 normal)
{
    //ピクセルの法線とライトの方向の内積を計算する。
    float t = dot(normal, lightDirection) * -1.0f;
    // 内積の結果を0～1に変換して影の部分にもグラデーションをつける
    t = t * 0.8f + 0.8f;
    //内積の結果が０より小さいときは０にする
    t = max(0.0f, t);

    //拡散反射光を計算する
    return lightColor * t;
}

// Phong鏡面反射光の計算
float3 CalcPhongSpecular(float3 lightDirection, float3 lightColor, float3 worldPos, float3 normal,float2 uv)
{
    // 反射ベクトルを求める
    float refVec = reflect(lightDirection, normal);

    // 光が当たったサーフェイスから視点に伸びるベクトルを求める
    float3 toEye = eyePos - worldPos;
    toEye = normalize(toEye);

    // 鏡面反射の強さを求める
    float t = dot(refVec, toEye);

    // 内積の結果が０より小さい時は０にする
    t = max(0.0f, t);

    // 鏡面反射の強さを絞る
    t = pow(t, 5.0f);

    // 鏡面反射光を求める
    float3 specularLig = lightColor * t;

    // スペキュラマップからスペキュラ反射の強さをサンプリング
    float specPower = g_speclarMap.Sample(g_sampler, uv).a;

    // 鏡面反射の強さを鏡面反射光に乗算する
    specularLig *= specPower * 2.0f;
     return lightColor * t;
}

// ディレクションライトによる反射光を計算
float3 CalcLigFromDirectionLight(SPSIn psIn)
{
    // ディレクションライトによるLambert拡散反射光を計算する
    float3 diffDirection = CalcLamberDiffuse(dirDirection, dirColor, psIn.normal);

    // ディレクションライトによるリムライトを計算する
    float3 limLight = CalcLigFromLimLight(dirDirection, dirColor, psIn.normal, psIn.normalInView);

    // ディレクションライトの最終的な反射光を返す
    return diffDirection + limLight;

}

// ポイントライトによる反射光を計算
float3 CalcLigFromPointLight(SPSIn psIn)
{
    // サーフェイスに入射するポイントライトの光の向きを求める
    float3 ligDir = psIn.worldPos - ptPosition;

    // 正規化
    ligDir = normalize(ligDir);

    // 拡散反射光を計算
    float3 diffPoint = CalcLamberDiffuse(
    ligDir,
    ptColor,
    psIn.normal
    );

    // 鏡面反射光を計算
    float3 specPoint = CalcPhongSpecular(
    ligDir,
    ptColor,
    psIn.worldPos,
    psIn.normal,
    psIn.uv
    );

    // 距離による影響率を計算する
    float3 distance = length(psIn.worldPos - ptPosition);

    // 影響率を距離によって変化させる
    float affect = 1.0f - 1.0f / ptRange * distance;

    // 影響率がマイナスにならないようにする
    affect = max(0.0f, affect);

    // 乗算して影響率の変化を指数関数的にする
    affect = pow(affect, ptRange);

    return (diffPoint + specPoint) * affect;

}

// スポットライトによる反射光を計算する
float3 CalcLigFromSpotLight(SPSIn psIn)
{
    // サーフェイスに入射するポイントライトの光の向きを計算する
    float3 ligDir = psIn.worldPos - spPosition;
    // 正規化して大きさ1のベクトルにする
    ligDir = normalize(ligDir);

    // 減衰なしのLambert拡散反射光を計算する
    float3 diffSpotLight = CalcLamberDiffuse(
        ligDir,
        ptColor,
        psIn.normal
    );

    // 減衰なしのPhong鏡面反射光を計算する
    float3 specSpotLight = CalcPhongSpecular(
        ligDir,
        ptColor,
        psIn.worldPos,
        psIn.normal,
        psIn.uv
    );

    // 距離による影響率を計算
    float3 distance = length(psIn.worldPos - spPosition);

    float affect = 1.0f - 1.0f / spRange * distance;

    if (affect < 0.0f)
    {
        affect = 0.0f;
    }

    affect = pow(affect, 3.0f);

    diffSpotLight *= affect;
    specSpotLight *= affect;

    float angle = dot(ligDir, spDirection);

    angle = abs(acos(angle));

    affect = 1.0f - 1.0f / spAngle * angle;

    if (affect < 0.0f)
    {
        affect = 0.0f;
    }

    affect = pow(affect, 0.8f);

    diffSpotLight *= affect;
    specSpotLight *= affect;

    return diffSpotLight + specSpotLight;
}



/** ディレクションライトによるリムライト */
float3 CalcLigFromLimLight(float3 lightDirection, float3 lightColor, float3 normal, float3 normalInView)
{
    float power1 = 1.0f - abs(dot(lightDirection, normal));

    float power2 = 1.0f - max(0.0f, normalInView.z * -1.0f);

    float limPower = power1 * power2;
    limPower = pow(limPower, 1.0f);

    return limPower * lightColor * 1.0;
}


/** 半球ライトを計算 */
float3 CalcLigFromHemiLight(SPSIn psIn)
{
    float t = dot(psIn.normal, groundNormal);

    t = (t + 1.0f) / 2.0f;

    float3 hemiLight = lerp(groundColor, skyColor, t);

    return hemiLight;
}

/** 法線を計算 */
float3 CalcNormal(float3 normal, float3 tangent, float3 biNormal, float2 uv)
{
    float3 binSpaceNormal = g_normalMap.SampleLevel(g_sampler, uv, 0.0f).xyz;
    binSpaceNormal = (binSpaceNormal * 2.0f) - 1.0f;

    float3 newNormal = tangent * binSpaceNormal.x + biNormal * binSpaceNormal.y + normal * binSpaceNormal.z;

    return newNormal;
}

/** シャドウマップ */
float CalcShadowMap(SPSIn psIn)
{
    float shadowAttn = 1.0f;
    float NdotL = dot(psIn.normal, -dirDirection);

    if (NdotL < 0.2f)
    {
        return shadowAttn;
    }

    float2 shadowMapUV = psIn.posInLVP.xy / psIn.posInLVP.w;
    shadowMapUV *= float2(0.5f, -0.5f);
    shadowMapUV += 0.5f;

    // ライトカメラの後ろ側は影なし
    if (psIn.posInLVP.w <= 0.0f)
        return shadowAttn;

    float zInLVP = psIn.posInLVP.z / psIn.posInLVP.w;
    float bias = 0.0005f;
    zInLVP -= bias;

    // ライトの視錐台深度範囲外は影なし
    if (zInLVP < 0.0f || zInLVP > 1.0f)
        return shadowAttn;

    if (shadowMapUV.x > 0.0f && shadowMapUV.x < 1.0f
        && shadowMapUV.y > 0.0f && shadowMapUV.y < 1.0f)
    {
        float texelSize = 1.0f / 4096.0f;

        float totalAttn = 0.0f;

        for (int x = -2; x <= 1; x++)
        {
            for (int y = -2; y <= 1; y++)
            {
                float2 offset = float2(x, y) * texelSize;
                float2 shadowValue = g_shadowMap.Sample(g_sampler, shadowMapUV + offset).xy;

                float attn = 1.0f;
                // shadowValue.r == 0 は未描画領域（影なし扱い）
                if (shadowValue.r > 0.001f && zInLVP > shadowValue.r)
                {
                    float md = zInLVP - shadowValue.r;
                    float depth_sq = shadowValue.r * shadowValue.r;
                    float variance = min(max(shadowValue.g - depth_sq, 0.0001f), 1.0f);
                    float md_scaled = md * 1.5f;
                    float lit_factor = variance / (variance + md_scaled * md_scaled);

                    if (lit_factor < 0.9f)
                    {
                        attn = 0.5f;
                    }
                }
                totalAttn += attn;
            }
        }

        shadowAttn = totalAttn / 16.0f;
    }
    return shadowAttn;
}
