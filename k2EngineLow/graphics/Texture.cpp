#include "k2EngineLowPreCompile.h"
#include "Texture.h"
#include <unordered_map>
#include <string>
#include <cwctype>

namespace nsK2EngineLow {

// 同一ファイルパスのDDSを何回ロードしてもD3D12 アタッチロードが1回で済むようにする。
// キャッシュはAddRef() を保持し、各 Texture インスタンスが自身の ref を持つ。
// プログラム終了時は OS/WDDM がGPU リソースを回収するため明示的解放は不要。
namespace {
    struct CachedTexture {
        ID3D12Resource* resource  = nullptr;
        bool            isCubemap = false;
    };
    std::unordered_map<std::wstring, CachedTexture> s_textureCache;

	// キャッシュキーの正規化：小文字化 + スラッシュ変換
	// TkmFileはテクスチャの拡張子を小文字の ".dds" に強制するが、プリロード時のパスには ".DDS" が使われることがある。
	// この正規化を行わないと、モデルテクスチャのキャッシュが常にミスヒットしてしまう。
    std::wstring NormalizeKey(const wchar_t* filePath)
    {
        std::wstring key(filePath);
        for (auto& c : key) {
            if (c == L'\\') c = L'/';
            else             c = static_cast<wchar_t>(towlower(c));
        }
        return key;
    }
}

	Texture::Texture(const wchar_t* filePath)
	{
		InitFromDDSFile(filePath);
	}
	Texture::~Texture()
	{
		Release();
	}
	void Texture::Release()
	{
		ReleaseD3D12Object(m_texture);
	}
	void Texture::InitFromDDSFile(const wchar_t* filePath)
	{
		Release();
		//DDSファイルからテクスチャをロード。
		LoadTextureFromDDSFile(filePath);

	}
	void Texture::InitFromD3DResource(ID3D12Resource* texture)
	{
		Release();
		m_texture = texture;
		m_texture->AddRef();
		m_textureDesc = m_texture->GetDesc();
	}
	void Texture::InitFromMemory(const char* memory, unsigned int size)
	{
		Release();
		//DDSファイルからテクスチャをロード。
		LoadTextureFromMemory(memory, size);

	}
	void Texture::LoadTextureFromMemory(const char* memory, unsigned int size
	)
	{
		Release();
		auto device = g_graphicsEngine->GetD3DDevice();
		DirectX::ResourceUploadBatch re(device);
		re.Begin();
		ID3D12Resource* texture;
		auto hr = DirectX::CreateDDSTextureFromMemoryEx(
			device,
			re,
			(const uint8_t*)memory,
			size,
			0,
			D3D12_RESOURCE_FLAG_NONE,
			0,
			&texture
		);
		re.End(g_graphicsEngine->GetCommandQueue());

		if (FAILED(hr)) {
			//テクスチャの作成に失敗しました。
			return;
		}

		m_texture = texture;
		m_textureDesc = m_texture->GetDesc();
	}
	void Texture::LoadTextureFromDDSFile(const wchar_t* filePath)
	{
		Release();

		// キャッシュヒット：D3D12へのアップロードをスキップし、AddRefのみ行う
		std::wstring key = NormalizeKey(filePath);
		auto it = s_textureCache.find(key);
		if (it != s_textureCache.end())
		{
			m_texture   = it->second.resource;
			m_isCubemap = it->second.isCubemap;
			m_texture->AddRef();
			m_textureDesc = m_texture->GetDesc();
			return;
		}

		auto device = g_graphicsEngine->GetD3DDevice();
		DirectX::ResourceUploadBatch re(device);
		re.Begin();
		ID3D12Resource* texture;
		auto hr = DirectX::CreateDDSTextureFromFileEx(
			device,
			re,
			filePath,
			0,
			D3D12_RESOURCE_FLAG_NONE,
			0,
			&texture,
			nullptr,
			&m_isCubemap
		);
		re.End(g_graphicsEngine->GetCommandQueue());

		if (FAILED(hr)) {
			return;
		}

		// キャッシュに登録（キャッシュ所有権のために追加でAddRefを行う）
		texture->AddRef();
		s_textureCache[key] = { texture, m_isCubemap };

		m_texture = texture;
		m_textureDesc = m_texture->GetDesc();
	}

	std::future<void> Texture::BatchPreloadToCacheAsync(const wchar_t* const* paths, int count)
	{
		// 背景スレッド（別スレッド）がデータを所有できるようにパスをコピーする
		std::vector<std::wstring> pathList(paths, paths + count);

		return std::async(std::launch::async, [pathList = std::move(pathList)]()
		{
			auto device = g_graphicsEngine->GetD3DDevice();
			DirectX::ResourceUploadBatch re(device);
			re.Begin();

			for (const auto& pathStr : pathList)
			{
				std::wstring key = NormalizeKey(pathStr.c_str());
				if (s_textureCache.count(key))
					continue;

				bool isCubemap = false;
				ID3D12Resource* texture = nullptr;
				auto hr = DirectX::CreateDDSTextureFromFileEx(
					device, re, pathStr.c_str(), 0,
					D3D12_RESOURCE_FLAG_NONE, 0,
					&texture, nullptr, &isCubemap
				);
				if (FAILED(hr) || !texture)
					continue;

				texture->AddRef();
				s_textureCache[key] = { texture, isCubemap };
			}

			// GPUの処理が完了するまで待機（メインスレッドではなく、背景スレッドをブロックする）
			auto gpuFuture = re.End(g_graphicsEngine->GetCommandQueue());
			gpuFuture.wait();
		});
	}

	void Texture::RegistShaderResourceView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, int bufferNo)
	{
		if (m_texture) {
			auto device = g_graphicsEngine->GetD3DDevice();
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = m_textureDesc.Format;
			if (m_isCubemap) {
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			}
			else {
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			}
			srvDesc.Texture2D.MipLevels = m_textureDesc.MipLevels;
			device->CreateShaderResourceView(m_texture, &srvDesc, descriptorHandle);
		}
	}
}