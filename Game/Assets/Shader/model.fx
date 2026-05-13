/*!
 * @brief	シンプルなモデルシェーダー。
 */

//定数
static const int NUM_DIRECTIONAL_LIGHT = 4; //ディレクションライトの数
static const int MAX_POINT_LIGHT = 32;      //ポイントライトの最大数
static const int MAX_SPOT_LIGHT = 32;       //スポットライトの最大数

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
   // float4 depth : SV_Target1;
};

//// ディレクションライト
//struct DirectionalLight
//{
//    float3 direction; //ライトの方向
//    //int castShadow; //影をキャストするか？
//    float3 color; //ライトのカラー
//};

//// ポイントライト
//struct PointLight
//{
//    float3 position; //座標
//    int isUse; //使用中フラグ
//    float3 color; //ライトのカラー
//    float range; //減衰パラメータ
//                        //xに影響範囲,yに影響率に累乗するパラメータ
//};

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
    
    //DirectionalLight directionalLights[NUM_DIRECTIONAL_LIGHT]; //ディレクションライト。
    //PointLight pointLights[MAX_POINT_LIGHT];//ポイントライト。
    
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
	//追加予定
}

////////////////////////////////////////////////
// シェーダーリソース
////////////////////////////////////////////////
// モデルテクスチャ
//Texture2D<float4> g_albedo : register(t0);		    //アルベドマップ
Texture2D<float4> g_texture : register(t0);             //モデルテクスチャ
Texture2D<float4> g_normalMap : register(t1);           //法線マップ
Texture2D<float4> g_speclarMap : register(t2);          //スペキュラマップ
//Texture2D<float4> g_aoMap : register(t10);            //AOマップ
Texture2D<float4> g_shadowMap : register(t10);           //シャドウマップ（SampleCmpLevelZero用にfloat型）

StructuredBuffer<float4x4> g_boneMatrix : register(t3);	//ボーン行列。

sampler g_sampler : register(s0);	                    //サンプラステート。

SamplerComparisonState g_shadowSampler : register(s1);  //シャドウマップ用の比較サンプラー



////////////////////////////////////////////////
// 関数宣言。
////////////////////////////////////////////////

// Lamber拡散反射光の計算
float3 CalcLamberDiffuse(float3 lightDirection, float3 lightColor, float3 normal);
float3 CalcPhongSpecular(float3 lightDirection, float3 lightColor, float3 worldPos, float3 normal);
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
    //psIn.posInLVP.z = length(worldPos.xyz - lightPos) / 1000.0f;


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
    //法線を計算
    //float3 normal = CalcNormal(psIn.normal, psIn.tangent, psIn.biNormal, psIn.uv);
    //環境光を計算
    //float3 ambient = ambientLight;
    //AOマップから環境光の強さをサンプリング
    //float ambientPower = g_aoMap.Sample(g_sampler, psIn.uv);
    //環境光の強さを環境光に乗算する
    //ambient *= ambientPower;
    
    // 各種ライトの反射光を足し算して最終的な反射光を求める
    float3 finalLig = directionLig + hemiLig;
    // 拡散反射光を計算する
    //finalLig += max(0.0f, dot(psIn.normal, -dirDirection)) * dirColor; //+ ambient;
    
    float4 finalColor = diffuseMap;
    // テクスチャカラーに求めた光を乗算して最終出力カラーを求める
    finalColor.xyz *= finalLig;
    
    float shadowAttn = 1.0f;
    if (isShadowReceiver == 1)
    {
        
       shadowAttn =  CalcShadowMap(psIn);
       
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
    t = t * 0.5f + 0.5f;
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
    //return specularLig * t;
     return lightColor * t;
}

// ディレクションライトによる反射光を計算
float3 CalcLigFromDirectionLight(SPSIn psIn)
{
    // ディレクションライトによるLambert拡散反射光を計算する
    float3 diffDirection = CalcLamberDiffuse(dirDirection, dirColor, psIn.normal);
    
    // ディレクションライトによるPhong鏡面反射光を計算する
    //float3 specDirection = CalcPhongSpecular(dirDirection, dirColor, psIn.worldPos, psIn.normal,psIn.uv);
    
    // ディレクションライトによるリムライトを計算する
    float3 limLight = CalcLigFromLimLight(dirDirection, dirColor, psIn.normal, psIn.normalInView);
    
    // ディレクションライトの最終的な反射光を返す
    return diffDirection + limLight;//diffDirection + specDirection;

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
    
    //// リムライトを計算する
    //float3 limLight = CalcLigFromLimLight(
    //ligDir,
    //spColor,
    //psIn.normal,
    //psIn.normalInView
    //);
    
    // 距離による影響率を計算する
    // ポイントライトとの距離を計算する
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
    // ポイントライトによるLambert拡散反射光とPhong鏡面反射光を計算する

    // サーフェイスに入射するポイントライトの光の向きを計算する
    float3 ligDir = psIn.worldPos - spPosition;
    // 正規化して大きさ1のベクトルにする
    ligDir = normalize(ligDir);

    // 減衰なしのLambert拡散反射光を計算する
    float3 diffSpotLight = CalcLamberDiffuse(
        ligDir,         // ライトの方向
        ptColor,        // ライトのカラー
        psIn.normal     // サーフェイスの法線
    );

    // 減衰なしのPhong鏡面反射光を計算する
    float3 specSpotLight = CalcPhongSpecular(
        ligDir,         // ライトの方向
        ptColor,        // ライトのカラー
        psIn.worldPos,  // サーフェイズのワールド座標
        psIn.normal,    // サーフェイズの法線
        psIn.uv
    );

    // 距離による影響率を計算
    // ポイントライトとの距離を計算
    float3 distance = length(psIn.worldPos - spPosition);

    // 影響率は距離に比例して減少
    float affect = 1.0f - 1.0f / spRange * distance;

    // 影響力がマイナスにならないように補正
    if (affect < 0.0f)
    {
        affect = 0.0f;
    }

    //影響を指数関数的に。今回は3乗。
    affect = pow(affect, 3.0f);

    // 拡散反射光と鏡面反射光に減衰率を乗算して影響を弱める
    diffSpotLight *= affect;
    specSpotLight *= affect;
    // limLight *= affect;

    // 入射角と射出方向の角度を求める(内積)
    float angle = dot(ligDir, spDirection);
    
    // dot()で求めた値をacos()に渡して角度を求める
    angle = abs(acos(angle));
    
    // 角度に比例して小さくなっていく影響率を計算
    affect = 1.0f - 1.0f / spAngle * angle;
    
    // 影響力がマイナスにならないように補正
    if (affect < 0.0f)
    {
        affect = 0.0f;
    }
    
     // 影響の仕方を指数関数的に。とりあえず0.5乗
    affect = pow(affect, 0.5f);
    
    // 角度による影響率を反射光に乗算して弱める
    diffSpotLight *= affect;
    specSpotLight *= affect;
    // limLight *= affect;

    return diffSpotLight + specSpotLight;
}



/** ディレクションライトによるリムライト */
float3 CalcLigFromLimLight(float3 lightDirection, float3 lightColor, float3 normal, float3 normalInView)
{   
    // リムライトの強さを求める
    // サーフェイスの法線と光の入射方向に依存するリムの強さを求める
    //float power1 = 1.0f - max(0.0f, dot(dirDirection, psIn.normal));
    float power1 = 1.0f - abs(dot(lightDirection, normal));
    
    // サーフェイスの法線と視線の方向に依存するリムの強さを求める
    //float power2 = 1.0f - max(0.0f, psIn.normalInView.z * -1.0f);
    float power2 = 1.0f - max(0.0f, normalInView.z * -1.0f);  // absをmaxに修正（裏面を無視する）
    
    // 最終的なリムの強さを求める
    float limPower = power1 * power2;
    // pow()を使用して、強さの変化を指数関数的にする
    // 指数を下げると幅が広がる、乗算係数で明るさを調整する
    limPower = pow(limPower, 1.0f);
    
    return limPower * lightColor * 2.0;
}


/** 半球ライトを計算 */
float3 CalcLigFromHemiLight(SPSIn psIn)
{
    //float3 directionLig = CalcLigFromDirectionLight(psIn);
    // サーフェイスの法線と地面の法線と内積を計算する
    float t = dot(psIn.normal, groundNormal);
    
    // 内積の結果を0～1の範囲に変換する
    t = (t + 1.0f) / 2.0f;
    
    // 地面色と天球色を補間率t で線形補完する
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
    // 戻り値の初期化（影なし = 1.0f）
    float shadowAttn = 1.0f;
    // ピクセルの法線とライトの方向がどれだけ同じ向きかを表す値
    float NdotL = dot(psIn.normal, -dirDirection);

    // ライトに対して浅い角度の面（ブロック側面など）は影判定スキップ
    if (NdotL < 0.2f)
    {
        return shadowAttn;
    }

    // ライトビュースクリーン空間からUV空間に座標変換
    float2 shadowMapUV = psIn.posInLVP.xy / psIn.posInLVP.w;
    shadowMapUV *= float2(0.5f, -0.5f);
    shadowMapUV += 0.5f;
    
    // ライトビュースクリーン空間でのZ値を計算
    float zInLVP = psIn.posInLVP.z / psIn.posInLVP.w;
    // ちらつき防止のオフセット
    float bias = 0.0005f;
    zInLVP -= bias;
    
    // UV座標がシャドウマップの範囲内のみ影判定
    if (shadowMapUV.x > 0.0f && shadowMapUV.x < 1.0f 
        && shadowMapUV.y > 0.0f && shadowMapUV.y < 1.0f)
    {
       // テクセルサイズ（4096解像度の場合）
        float texelSize = 1.0f / 4096.0f;

        float totalAttn = 0.0f;

        // 4×4の16点サンプリング
        for (int x = -2; x <= 1; x++)
        {
            for (int y = -2; y <= 1; y++)
            {
                float2 offset = float2(x, y) * texelSize;
                float2 shadowValue = g_shadowMap.Sample(g_sampler, shadowMapUV + offset).xy;
    
                float attn = 1.0f;
                if (zInLVP > shadowValue.r)
                {
                    float md = zInLVP - shadowValue.r;
                    float depth_sq = shadowValue.r * shadowValue.r;
                    float variance = min(max(shadowValue.g - depth_sq, 0.0001f), 1.0f);
                    // md はクリップ空間（0〜1）の差なので非常に小さい
                    // スケールを掛けて増幅することで影を濃くする
                    float md_scaled = md * 1.5f;
                    float lit_factor = variance / (variance + md_scaled * md_scaled);

                    // lit_factor=0.0f（完全に影）→ 0.5f（影の色）
                    // lit_factor=1.0f（完全に明るい）→ 1.0f
                    // attn = lerp(0.15f, 1.0f, lit_factor);
                    // しきい値で二値判定して固定値を使う
                    if (lit_factor < 0.9f)
                    {
                        attn = 0.5f;  // 常に固定の濃さ
                    }
                }
                totalAttn += attn;
            }
        }

        // 16点の平均
        shadowAttn = totalAttn / 16.0f;
    }
    return shadowAttn;
}