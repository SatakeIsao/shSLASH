/*!
 * @brief	シンプルなモデルシェーダー。
 */

////////////////////////////////////////////////
// 構造体
////////////////////////////////////////////////
//スキニング用の頂点データをひとまとめ。
struct SSkinVSIn
{
	int4   Indices  	: BLENDINDICES0;
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

////////////////////////////////////////////////
// シェーダーリソース
////////////////////////////////////////////////
// モデルテクスチャ
Texture2D<float4> g_albedo : register(t0);				//アルベドマップ
//Texture2D<float4> g_texture : register(t0);             //モデルテクスチャ
//Texture2D<float4> g_normalMap : register(t1);           //法線マップ
//Texture2D<float4> g_speclarMap : register(t2);          //スペキュラマップ
//Texture2D<float4> g_aoMap : register(t10);              //AOマップ
Texture2D<float4> g_shadowMap : register(t11);          //シャドウマップ

StructuredBuffer<float4x4> g_boneMatrix : register(t3);	//ボーン行列。

sampler g_sampler : register(s0);	//サンプラステート。



////////////////////////////////////////////////
// 関数宣言。
////////////////////////////////////////////////

//// Lamber拡散反射光の計算
//float3 CalcLamberDiffuse(float3 lightDirection, float3 lightColor, float3 normal);
//float3 CalcPhongSpecular(float3 lightDirection, float3 lightColor, float3 worldPos, float3 normal);
//float3 CalcLigFromDirectionLight(SPSIn psIn);
//float3 CalcLigFromPointLight(SPSIn psIn);
//float3 CalcLigFromSpotLight(SPSIn psIn);
//float3 CalcLigFromLimLight(float3 lightDirection, float3 lightColor, float3 normal, float3 normalInView);
//float3 CalcLigFromHemiLight(SPSIn psIn);
//float3 CalcNormal(float3 normal, float3 tangent, float3 biNormal, float2 uv);

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
    //シャドウマップ描画用の頂点シェーダーを実装
    SPSIn psIn;
    float4x4 worldMatrix;
    if (!hasSkin)
    {
        worldMatrix = mWorld;

    }
    else
    {
        worldMatrix = CalcSkinMatrix(vsIn.skinVert);

    }
    
    psIn.pos = mul(worldMatrix, vsIn.pos); //モデルの頂点をワールド座標系に変換
    psIn.worldPos = psIn.pos;
    psIn.pos = mul(mView, psIn.pos); //ワールド座標系からカメラ座標系に変換
    psIn.pos = mul(mProj, psIn.pos); //カメラ座標系からスクリーン座標系に変換

    psIn.normal = mul(worldMatrix, vsIn.normal); //法線を回転させる。
    
    psIn.tangent = normalize(mul(worldMatrix, vsIn.tangent)); //接ベクトルをワールド空間に変換する
    psIn.biNormal = normalize(mul(worldMatrix, vsIn.biNormal)); //従ベクトルをワールド空間に変換する
    
    psIn.uv = vsIn.uv;

    psIn.normalInView = mul(mView, psIn.normal); //カメラ空間の法線を求める
    
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
/// シャドウマップ描画用のピクセルシェーダー。
/// </summary>
float4 PSMain( SPSIn psIn ) : SV_Target0
{
    //return float4(0.5f, 0.5f, 0.5f, 1.0f);
    
    //シャドウマップにz値を書き込む
    return float4(psIn.pos.z, psIn.pos.z, psIn.pos.z, 1.0f);
}
