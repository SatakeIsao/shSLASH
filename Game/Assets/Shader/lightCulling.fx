/*!
 * @brief タイルベースライトカリング コンピュートシェーダー
 * 16x16 ピクセルのタイルごとに影響するポイントライトを事前計算し、
 * 後段のディファードライティングパスで使用するインデックスリストを出力する。
 */

#define TILE_WIDTH  16
#define TILE_HEIGHT 16
#define TILE_SIZE   (TILE_WIDTH * TILE_HEIGHT)
#define MAX_TBDR_POINT_LIGHT 1000

// ポイントライトデータ（CPU 側の TBDRPointLight に対応）
struct TBDRPointLight
{
    float4 position; // xyz=ワールド座標, w=pad0（C++ alignas(16) の 4byte パディング）
    float3 color;
    float  range;
};

// b0: カメラ・スクリーンパラメーター
cbuffer cbCameraParam : register(b0)
{
    float4x4 mView;       // ビュー行列（worldPos → カメラ空間変換用）
    float4x4 mtxProj;     // プロジェクション行列（錐台面計算用）
    float4x4 mtxProjInv;  // 未使用（将来用）
    float4   screenParam; // x=near, y=far, z=width, w=height
};

// b1: ライト数パラメーター
cbuffer TBDRParams : register(b1)
{
    int   numPointLight;
    float _pad0;
    float _pad1;
    float _pad2;
};

// t0: G-Buffer ワールド座標テクスチャ（w=0:ジオメトリなし、w=1:あり）
Texture2D<float4> worldPosTexture : register(t0);

// t1: ポイントライトデータ（StructuredBuffer<TBDRPointLight>）
StructuredBuffer<TBDRPointLight> lightBuffer : register(t1);

// u0: タイル別ライトインデックスリスト出力（uint）
RWStructuredBuffer<uint> rwLightIndices : register(u0);

// グループ共有メモリ
groupshared uint sMinZ;                                  // タイル最小深度（asuint）
groupshared uint sMaxZ;                                  // タイル最大深度（asuint）
groupshared uint sTileLightIndices[MAX_TBDR_POINT_LIGHT]; // タイルに影響するライト番号
groupshared uint sTileNumLights;                          // タイルに影響するライト数

/*!
 * @brief タイルの視錐台 6 面を求める（ビュー空間）
 */
void GetTileFrustumPlane(out float4 frustumPlanes[6], uint3 groupId)
{
    float minTileZ = asfloat(sMinZ);
    float maxTileZ = asfloat(sMaxZ);

    float2 tileScale = screenParam.zw * rcp(float(2 * TILE_WIDTH));
    float2 tileBias  = tileScale - float2(groupId.xy);

    float4 c1 = float4(mtxProj._11 * tileScale.x, 0.0f, tileBias.x, 0.0f);
    float4 c2 = float4(0.0f, -mtxProj._22 * tileScale.y, tileBias.y, 0.0f);
    float4 c4 = float4(0.0f, 0.0f, 1.0f, 0.0f);

    frustumPlanes[0] = c4 - c1; // Right
    frustumPlanes[1] = c4 + c1; // Left
    frustumPlanes[2] = c4 - c2; // Top
    frustumPlanes[3] = c4 + c2; // Bottom
    frustumPlanes[4] = float4(0.0f, 0.0f,  1.0f, -minTileZ);
    frustumPlanes[5] = float4(0.0f, 0.0f, -1.0f,  maxTileZ);

    // 4 側面のみ正規化
    [unroll]
    for (uint i = 0; i < 4; ++i)
    {
        frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));
    }
}

[numthreads(TILE_WIDTH, TILE_HEIGHT, 1)]
void CSMain(
    uint3 groupId          : SV_GroupID,
    uint3 dispatchThreadId : SV_DispatchThreadID,
    uint3 groupThreadId    : SV_GroupThreadID)
{
    uint groupIndex = groupThreadId.y * TILE_WIDTH + groupThreadId.x;

    // ---- 1. グループ共有メモリ初期化 ----
    if (groupIndex == 0)
    {
        sTileNumLights = 0;
        sMinZ = 0x7F7FFFFF; // float max（InterlockedMin で実際の最小値に置き換え）
        sMaxZ = 0;          // float 0.0（InterlockedMax で実際の最大値に置き換え）
    }
    GroupMemoryBarrierWithGroupSync();

    // ---- 2. G-Buffer からビュー空間 Z を取得 ----
    uint2 frameUV = dispatchThreadId.xy;
    float4 worldPosSamp = worldPosTexture.Load(uint3(frameUV, 0));

    if (worldPosSamp.w >= 0.5f) // ジオメトリあり
    {
        float4 viewPos = mul(mView, float4(worldPosSamp.xyz, 1.0f));
        float  viewZ   = viewPos.z;
        if (viewZ > 0.0f)
        {
            InterlockedMin(sMinZ, asuint(viewZ));
            InterlockedMax(sMaxZ, asuint(viewZ));
        }
    }
    GroupMemoryBarrierWithGroupSync();

    // ジオメトリなしタイルはフルレンジを使用（保守的カリング）
    if (groupIndex == 0 && sMaxZ == 0)
    {
        sMinZ = asuint(screenParam.x); // near
        sMaxZ = asuint(screenParam.y); // far
    }
    GroupMemoryBarrierWithGroupSync();

    // ---- 3. タイルの視錐台 6 面を計算 ----
    float4 frustumPlanes[6];
    GetTileFrustumPlane(frustumPlanes, groupId);

    // ---- 4. ライトと錐台の当たり判定（並列処理）----
    for (uint lightIndex = groupIndex; lightIndex < (uint)numPointLight; lightIndex += TILE_SIZE)
    {
        TBDRPointLight lig = lightBuffer[lightIndex];

        // ワールド座標 → ビュー座標
        float3 posInView = mul(mView, float4(lig.position.xyz, 1.0f)).xyz;

        bool inFrustum = true;
        [unroll]
        for (uint i = 0; i < 6; ++i)
        {
            float d = dot(frustumPlanes[i], float4(posInView, 1.0f));
            inFrustum = inFrustum && (d >= -lig.range);
        }

        if (inFrustum)
        {
            uint listIndex;
            InterlockedAdd(sTileNumLights, 1, listIndex);
            if (listIndex < MAX_TBDR_POINT_LIGHT)
            {
                sTileLightIndices[listIndex] = lightIndex;
            }
        }
    }
    GroupMemoryBarrierWithGroupSync();

    // ---- 5. 結果をグローバルバッファへ出力 ----
    // ストライドは MAX_TBDR_POINT_LIGHT で固定（UAV のアロケートサイズと一致させる）
    uint numCellX  = ((uint)screenParam.z + TILE_WIDTH  - 1) / TILE_WIDTH;
    uint tileIndex = frameUV.x / TILE_WIDTH + (frameUV.y / TILE_HEIGHT) * numCellX;
    uint lightStart = (uint)MAX_TBDR_POINT_LIGHT * tileIndex;

    uint numLightsInTile = min(sTileNumLights, (uint)MAX_TBDR_POINT_LIGHT);
    for (uint li = groupIndex; li < numLightsInTile; li += TILE_SIZE)
    {
        rwLightIndices[lightStart + li] = sTileLightIndices[li];
    }

    // センチネル値で終端を示す（ライト数が最大値未満の場合のみ）
    if (groupIndex == 0 && numLightsInTile < (uint)MAX_TBDR_POINT_LIGHT)
    {
        rwLightIndices[lightStart + numLightsInTile] = 0xffffffff;
    }
}
