#include "k2EngineLowPreCompile.h"
#include "RootSignature.h"
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace nsK2EngineLow {
	enum {
		enDescriptorHeap_CB,
		enDescriptorHeap_SRV,
		enDescriptorHeap_UAV,
		enNumDescriptorHeap
	};

namespace {
	// シリアライズされたBlobの内容（バイナリ）をキーにしてルートシグネチャをキャッシュする。
	// 引数が同一であれば、シリアライズされるBlobの内容も同一になり、同じ ID3D12RootSignature* を生成する。
	// これによりマテリアル間でPSO（パイプライン状態オブジェクト）記述子の比較が可能になり、PSOキャッシュのヒット率が向上する。
	struct RSKey {
		std::vector<uint8_t> blob;
		bool operator==(const RSKey& o) const { return blob == o.blob; }
	};
	struct RSKeyHash {
		size_t operator()(const RSKey& k) const {
			size_t h = 14695981039346656037ULL;
			for (auto b : k.blob) { h ^= b; h *= 1099511628211ULL; }
			return h;
		}
	};
	std::unordered_map<RSKey, ID3D12RootSignature*, RSKeyHash> s_rsCache;
}

	RootSignature::~RootSignature()
	{
		Release();
	}
	void RootSignature::Release()
	{
		ReleaseD3D12Object(m_rootSignature);
	}
	bool RootSignature::Init(
		D3D12_STATIC_SAMPLER_DESC* samplerDescArray,
		int numSampler,
		UINT maxCbvDescriptor,
		UINT maxSrvDescriptor,
		UINT maxUavDescritor,
		UINT offsetInDescriptorsFromTableStartCB,
		UINT offsetInDescriptorsFromTableStartSRV,
		UINT offsetInDescriptorsFromTableStartUAV
	)
	{
		Release();
		auto d3dDevice = g_graphicsEngine->GetD3DDevice();

		CD3DX12_DESCRIPTOR_RANGE1 ranges[enNumDescriptorHeap];
		CD3DX12_ROOT_PARAMETER1 rootParameters[enNumDescriptorHeap];

		ranges[enDescriptorHeap_CB].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, maxCbvDescriptor, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, offsetInDescriptorsFromTableStartCB);
		rootParameters[enDescriptorHeap_CB].InitAsDescriptorTable(1, &ranges[enDescriptorHeap_CB]);

		ranges[enDescriptorHeap_SRV].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, maxSrvDescriptor, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, offsetInDescriptorsFromTableStartSRV);
		rootParameters[enDescriptorHeap_SRV].InitAsDescriptorTable(1, &ranges[enDescriptorHeap_SRV]);

		ranges[enDescriptorHeap_UAV].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, maxUavDescritor, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, offsetInDescriptorsFromTableStartUAV);
		rootParameters[enDescriptorHeap_UAV].InitAsDescriptorTable(1, &ranges[enDescriptorHeap_UAV]);

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, numSampler, samplerDescArray, rootSignatureFlags);
		Microsoft::WRL::ComPtr<ID3DBlob> signature;
		Microsoft::WRL::ComPtr<ID3DBlob> error;
		D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

		// シリアライズされたバイトコードをキーにキャッシュを検索：同じパラメータであれば同じバイト列になり、同じポインタを返す
		RSKey key;
		const auto* p = static_cast<const uint8_t*>(signature->GetBufferPointer());
		key.blob.assign(p, p + signature->GetBufferSize());
		auto it = s_rsCache.find(key);
		if (it != s_rsCache.end()) {
			m_rootSignature = it->second;
			m_rootSignature->AddRef();
			return true;
		}

		auto hr = d3dDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
		if (FAILED(hr)) {
			return false;
		}
		m_rootSignature->AddRef();
		s_rsCache[key] = m_rootSignature;
		return true;
	}
	bool RootSignature::Init(
		D3D12_FILTER samplerFilter,
		D3D12_TEXTURE_ADDRESS_MODE textureAdressModeU,
		D3D12_TEXTURE_ADDRESS_MODE textureAdressModeV,
		D3D12_TEXTURE_ADDRESS_MODE textureAdressModeW,
		UINT maxCbvDescriptor,
		UINT maxSrvDescriptor,
		UINT maxUavDescritor
	)
	{


		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = samplerFilter;
		sampler.AddressU = textureAdressModeU;
		sampler.AddressV = textureAdressModeV;
		sampler.AddressW = textureAdressModeW;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		return Init(&sampler, 1, maxCbvDescriptor, maxSrvDescriptor, maxUavDescritor);
	}

	bool RootSignature::Init(Shader& shader)
	{
		// シェーダーからルートシグネチャの情報を取得
		ID3DBlob* sig = nullptr;
		auto shaderBlob = shader.GetCompiledBlob();

		auto hr = D3DGetBlobPart(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
			D3D_BLOB_ROOT_SIGNATURE, 0, &sig);
		// ルートシグネチャの生成
		auto d3dDevice = g_graphicsEngine->GetD3DDevice();
		hr = d3dDevice->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(),
			IID_PPV_ARGS(&m_rootSignature));
		return hr == S_OK;
	}
}