#include "k2EngineLowPreCompile.h"
#include "Shader.h"
#include <stierr.h>
#include <sstream>
#include <fstream>
#include <atlbase.h>
#include <unordered_map>
#include <mutex>
#include <future>
#include <vector>
#include <array>

namespace nsK2EngineLow {

	namespace {
		const char* g_vsShaderModelName = "vs_5_0";
		const char* g_psShaderModelName = "ps_5_0";
		const char* g_csShaderModelName = "cs_5_0";

		// D3DCompileFromFile の呼び出しには 1回あたり 50〜500ms かかるため、コンパイル済みのバイナリをキャッシュする。
		// これにより、同一の (filePath, entryFunc, shaderModel) の組み合わせはプロセス内で1度だけコンパイルされる。
		// スレッドセーフ: D3DCompileFromFile はCPUのみで動作しスレッドセーフ。キャッシュ操作はミューテックスで保護。
		struct ShaderKey {
			std::string filePath;
			std::string entryFunc;
			std::string shaderModel;
			bool operator==(const ShaderKey& o) const {
				return filePath == o.filePath && entryFunc == o.entryFunc && shaderModel == o.shaderModel;
			}
		};
		struct ShaderKeyHash {
			size_t operator()(const ShaderKey& k) const {
				size_t h = std::hash<std::string>{}(k.filePath);
				h ^= std::hash<std::string>{}(k.entryFunc)   + 0x9e3779b9u + (h << 6) + (h >> 2);
				h ^= std::hash<std::string>{}(k.shaderModel) + 0x9e3779b9u + (h << 6) + (h >> 2);
				return h;
			}
		};
		std::mutex s_shaderCacheMutex;
		std::unordered_map<ShaderKey, ID3DBlob*, ShaderKeyHash> s_shaderCache;
	}

	Shader::~Shader()
	{
		Release();
	}
	void Shader::Release()
	{
		ReleaseD3D12Object(m_blob);
		ReleaseD3D12Object(m_dxcBlob);
	}
	void Shader::Load(const char* filePath, const char* entryFuncName, const char* shaderModel)
	{
		Release();

		ShaderKey key{ filePath, entryFuncName, shaderModel };

		// ファストパス：キャッシュにヒットした場合はコンパイルを行わず、キャッシュされたバイナリを返す
		{
			std::lock_guard<std::mutex> lk(s_shaderCacheMutex);
			auto it = s_shaderCache.find(key);
			if (it != s_shaderCache.end()) {
				m_blob = it->second;
				m_blob->AddRef();
				m_isInited = true;
				return;
			}
		}

		// ロックを保持せずにコンパイルを実行 — D3DCompileFromFile はCPUのみで動作しスレッドセーフ
		ID3DBlob* compiledBlob = nullptr;
		ID3DBlob* errorBlob    = nullptr;
#ifdef K2_DEBUG
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		wchar_t wfxFilePath[256] = { L"" };
		mbstowcs(wfxFilePath, filePath, 256);

		auto hr = D3DCompileFromFile(wfxFilePath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryFuncName, shaderModel, compileFlags, 0, &compiledBlob, &errorBlob);

		if (FAILED(hr)) {
			if (hr == STIERR_OBJECTNOTFOUND) {
				std::wstring errorMessage = L"Shader file not found: ";
				errorMessage += wfxFilePath;
				MessageBoxW(nullptr, errorMessage.c_str(), L"Error", MB_OK);
			}
			if (errorBlob) {
				static char errorMessage[10 * 1024];
				sprintf_s(errorMessage, "filePath : %ws, %s", wfxFilePath, (char*)errorBlob->GetBufferPointer());
				MessageBoxA(NULL, errorMessage, "Shader compile error", MB_OK);
				return;
			}
			return;
		}

		// ロックを取得して再度チェック：他スレッドが先に同じシェーダーのコンパイルを終えている可能性がある
		{
			std::lock_guard<std::mutex> lk(s_shaderCacheMutex);
			auto it = s_shaderCache.find(key);
			if (it != s_shaderCache.end()) {
				// 他のスレッドが先に登録していた場合、キャッシュされた blob を使用し、自身がコンパイルしたものは破棄する
				m_blob = it->second;
				m_blob->AddRef();
				compiledBlob->Release();
			} else {
				// 自身が最初の登録者の場合、キャッシュ所有権のために AddRef を追加で行い、キャッシュに登録する
				m_blob = compiledBlob;   // 参照カウント = 1 (この Shader インスタンスが所有)
				m_blob->AddRef();        // 参照カウント = 2 (キャッシュ用に 1 加算)
				s_shaderCache[key] = m_blob;
			}
		}

		m_isInited = true;
	}

	std::future<void> Shader::PrecompileAsync(std::vector<std::array<std::string, 3>> list)
	{
		return std::async(std::launch::async, [list = std::move(list)]()
		{
			for (const auto& entry : list)
			{
				Shader s;
				s.Load(entry[0].c_str(), entry[1].c_str(), entry[2].c_str());
				// s.~Shader() の呼び出しでこのインスタンスの blob 参照は解放されるが、キャッシュ側の参照は独立して維持される
			}
		});
	}
	void Shader::LoadPS(const char* filePath, const char* entryFuncName)
	{
		Load(filePath, entryFuncName, g_psShaderModelName);
	}
	void Shader::LoadVS(const char* filePath, const char* entryFuncName)
	{
		Load(filePath, entryFuncName, g_vsShaderModelName);
	}
	void Shader::LoadCS(const char* filePath, const char* entryFuncName)
	{
		Load(filePath, entryFuncName, g_csShaderModelName);
	}
	void Shader::LoadRaytracing(const wchar_t* filePath)
	{
		std::ifstream shaderFile(filePath);
		if (shaderFile.good() == false) {
			std::wstring errormessage = L"シェーダーファイルのオープンに失敗しました。\n";
			errormessage += filePath;
			MessageBoxW(nullptr, errormessage.c_str(), L"エラー", MB_OK);
			std::abort();
		}

		std::stringstream strStream;
		strStream << shaderFile.rdbuf();
		std::string shader = strStream.str();
		// シェーダーのテキストファイルからBLOBを作成する。
		CComPtr<IDxcLibrary> dxclib;
		auto hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&dxclib));
		if (FAILED(hr)) {
			MessageBox(nullptr, L"DXCLIBの作成に失敗しました。", L"エラー", MB_OK);
			std::abort();
		}
		CComPtr< IDxcIncludeHandler> includerHandler;
		hr = dxclib->CreateIncludeHandler(&includerHandler);
		if (FAILED(hr)) {
			MessageBox(nullptr, L"CreateIncludeHandlerに失敗しました。", L"エラー", MB_OK);
			std::abort();
		}

		// dxcコンパイラの作成。
		CComPtr<IDxcCompiler> dxcCompiler;
		hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
		if (FAILED(hr)) {
			MessageBox(nullptr, L"dxcコンパイラの作成に失敗しました。", L"エラー", MB_OK);
			std::abort();
		}
		// ソースコードからBLOBを作成する。
		uint32_t codePage = CP_UTF8;
		CComPtr< IDxcBlobEncoding> sourceBlob;
		hr = dxclib->CreateBlobFromFile(filePath, &codePage, &sourceBlob);
		if (FAILED(hr)) {
			MessageBox(nullptr, L"シェーダーソースのBlobの作成に失敗しました。", L"エラー", MB_OK);
			std::abort();
		}

		CComPtr<IDxcIncludeHandler> dxcIncludeHandler;
		dxclib->CreateIncludeHandler(&dxcIncludeHandler);
		const wchar_t* args[] = {
			L"-I Assets\\shader",
		};
		// コンパイル
		CComPtr<IDxcOperationResult> result;
		hr = dxcCompiler->Compile(
			sourceBlob,			// ソースコードのBlob
			filePath,			// ソースファイル名
			L"",				// エントリーポイント名（ライブラリの場合は空文字）
			L"lib_6_3",			// ターゲットプロファイル
			args, 1,			// コンパイル引数（ポインタと引数個数）
			nullptr, 0,			// マクロ定義（ポインタと定義個数）
			dxcIncludeHandler,	// インクルードハンドラー
			&result);			// コンパイル結果の格納先
		if (SUCCEEDED(hr)) {
			result->GetStatus(&hr);
		}

		if (FAILED(hr))
		{
			if (result)
			{
				CComPtr<IDxcBlobEncoding> errorsBlob;
				hr = result->GetErrorBuffer(&errorsBlob);
				if (SUCCEEDED(hr) && errorsBlob)
				{
					std::string errormessage = "Compilation failed with errors:\n%hs\n";
					errormessage += (const char*)errorsBlob->GetBufferPointer();
					MessageBoxA(nullptr, errormessage.c_str(), "エラー", MB_OK);

				}
			}
			// コンパイルエラーのハンドリング処理...
		}
		else {
			result->GetResult(&m_dxcBlob);
		}
	}
}
