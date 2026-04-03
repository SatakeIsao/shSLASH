/**
 * BoundingVolume.h
 * MeshからAABBを作成するクラス
 */
#pragma once


namespace app
{
	namespace collision
	{
		struct Bounds
		{
			Vector3 minPoint = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
			Vector3 maxPoint = Vector3(FLT_MIN, FLT_MIN, FLT_MIN);
			//
			void Compute(Model& model)
			{
				const auto& tkmFile = model.GetTkmFile();
				const auto& meshParts = tkmFile.GetMeshParts();
				//メッシュを一つづつ調べていく。
				for (const auto& mesh : meshParts) {
					for (const auto& vertex : mesh.vertexBuffer) {
						const auto& pos = vertex.pos;
						// 最小値から比較
						minPoint.x = min(minPoint.x, pos.x);
						minPoint.y = min(minPoint.y, pos.y);
						minPoint.z = min(minPoint.z, pos.z);
						// 最大値から比較
						maxPoint.x = max(maxPoint.x, pos.x);
						maxPoint.y = max(maxPoint.y, pos.y);
						maxPoint.z = max(maxPoint.z, pos.z);
					}
				}
			}
		};
	}
}