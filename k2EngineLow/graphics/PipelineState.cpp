#include "k2EngineLowPreCompile.h"
#include "PipelineState.h"
#include <unordered_map>
#include <cstring>

namespace nsK2EngineLow {

namespace {
	// PSOキャッシュ: CreateGraphicsPipelineState の呼び出しには 1回あたり 5〜50ms かかるため、同じ記述子（desc）であればオブジェクトを再利用する。
	// キーは desc 全体のバイト単位のハッシュ値。Material::InitPipelineState 内で使用前にゼロ初期化されるため、未初期化のパディングバイトは存在しない。
	// Shader::Load が (ファイル, エントリー, モデル) ごとに Blob をキャッシュするようになったため、シェーダーのバイトコードポインタは不変（安定）である。
    struct PSOKey {
        size_t hash;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
        bool operator==(const PSOKey& o) const {
            return std::memcmp(&desc, &o.desc, sizeof(desc)) == 0;
        }
    };
    struct PSOKeyHash {
        size_t operator()(const PSOKey& k) const { return k.hash; }
    };
    std::unordered_map<PSOKey, ID3D12PipelineState*, PSOKeyHash> s_psoCache;

    size_t HashBytes(const void* data, size_t size)
    {
        const auto* p = static_cast<const unsigned char*>(data);
        size_t h = 14695981039346656037ULL;
        for (size_t i = 0; i < size; ++i) {
            h ^= p[i];
            h *= 1099511628211ULL;
        }
        return h;
    }
}

	PipelineState::~PipelineState()
	{
		Release();
	}
	void PipelineState::Release()
	{
		ReleaseD3D12Object(m_pipelineState);
	}
	void PipelineState::Init(D3D12_GRAPHICS_PIPELINE_STATE_DESC desc)
	{
		Release();

		PSOKey key;
		key.desc = desc;
		key.hash = HashBytes(&desc, sizeof(desc));

		auto it = s_psoCache.find(key);
		if (it != s_psoCache.end()) {
			m_pipelineState = it->second;
			m_pipelineState->AddRef();
			return;
		}

		auto d3dDevice = g_graphicsEngine->GetD3DDevice();
		auto hr = d3dDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_pipelineState));
		if (FAILED(hr)) {
			MessageBoxA(nullptr, "Failed to create pipeline state.\n", "Error", MB_OK);
			std::abort();
		}

		m_pipelineState->AddRef();
		s_psoCache[key] = m_pipelineState;
	}
	void PipelineState::Init(D3D12_COMPUTE_PIPELINE_STATE_DESC desc)
	{
		Release();
		auto d3dDevice = g_graphicsEngine->GetD3DDevice();
		auto hr = d3dDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(&m_pipelineState));
		if (FAILED(hr)) {
			MessageBoxA(nullptr, "Failed to create pipeline state.\n", "Error", MB_OK);
			std::abort();
		}
	}
}
