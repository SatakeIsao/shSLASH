#pragma once


namespace nsK2EngineLow {
	class Material;
	namespace raytracing {
		using ID3D12DescriptorHeapPtr = CComPtr<ID3D12DescriptorHeap>;

		struct AccelerationStructureBuffers {
			ID3D12Resource* pScratch = nullptr;
			ID3D12Resource* pResult = nullptr;
			ID3D12Resource* pInstanceDesc = nullptr;
		};

		/// <summary>
		/// レイトレのインスタンスデータ。
		/// </summary>
		struct Instance {
			D3D12_RAYTRACING_GEOMETRY_DESC geometoryDesc;	//ジオメトリ情報。
			RWStructuredBuffer m_vertexBufferRWSB;			//頂点バッファ。
			RWStructuredBuffer m_indexBufferRWSB;			//インデックスバッファ。
			Material* m_material = nullptr;					//マテリアル。
		};
		using InstancePtr = std::unique_ptr<Instance>;
	}//namespace raytracing
}//namespace nsK2EngineLow 