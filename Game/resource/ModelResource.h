#pragma once

#include "ResourceManager.h"

namespace nsK2EngineLow
{
	class TkmFile;
	class TkaFile;
	class TksFile;
}

namespace app
{
	namespace resource
	{
		// ---------------------------------------------------------------
		// TkmResource / TkmLoader
		// ワーカースレッド: .tkm をCPUメモリにロードする。
		// AutoFinalize（メインスレッド）: エンジンバンクに登録し、
		// ModelRender::Init() がファイルI/Oをスキップできるようにする。
		// ---------------------------------------------------------------
		class TkmResource : public IResource
		{
			friend class TkmLoader;

		public:
			void AutoFinalize() override
			{
				if (m_tkmFile)
				{
					/**
					 * TODO:  
					 * 今は問題ないけど、 読み込み処理の順番によってマルチスレッドの問題が出てくる
					 * K2Engineはシングル
					 * Shared使えるのがベスト
					 * Bankを使わない※シングル専用なので
					 * リソースマネージャで管理してあげる
					 * ロックかけずにマルチスレッド使う必要、ロックかける必要性
					 * 推奨スペック等の詳細説明入れておく（5GB必要等）➤PFに書いておく
					 */
					g_engine->RegistTkmFileToBank(m_filePath.c_str(), m_tkmFile.get());
					m_tkmFile.release();
				}
			}

		private:
			std::string m_filePath;
			std::unique_ptr<nsK2EngineLow::TkmFile> m_tkmFile;
		};

		class TkmLoader : public ResourceLoader<TkmResource>
		{
		public:
			bool LoadImpl(TkmResource& resource, const std::string& key) override
			{
				resource.m_filePath = key;
				/** TODO: マルチスレッド対応できていない➤ここはシングルだと問題なしだが */
				if (g_engine->GetTkmFileFromBank(key.c_str()) != nullptr)
				{
					return true;
				}
				auto tkmFile = std::make_unique<nsK2EngineLow::TkmFile>();
				if (!tkmFile->Load(key.c_str(), false))
				{
					return false;
				}
				resource.m_tkmFile = std::move(tkmFile);
				return true;
			}
		};


		// ---------------------------------------------------------------
		// TkaResource / TkaLoader
		// ワーカースレッド: .tka をCPUメモリにロードする。
		// AutoFinalize（メインスレッド）: エンジンバンクに登録し、
		// AnimationClip::Load() がファイルI/Oをスキップできるようにする。
		// ---------------------------------------------------------------
		class TkaResource : public IResource
		{
			friend class TkaLoader;

		public:
			void AutoFinalize() override
			{
				if (m_tkaFile)
				{
					g_engine->RegistTkaFileToBank(m_filePath.c_str(), m_tkaFile.get());
					m_tkaFile.release();
				}
			}

		private:
			std::string m_filePath;
			std::unique_ptr<nsK2EngineLow::TkaFile> m_tkaFile;
		};

		class TkaLoader : public ResourceLoader<TkaResource>
		{
		public:
			bool LoadImpl(TkaResource& resource, const std::string& key) override
			{
				resource.m_filePath = key;
				if (g_engine->GetTkaFileFromBank(key.c_str()) != nullptr)
				{
					return true;
				}
				auto tkaFile = std::make_unique<nsK2EngineLow::TkaFile>();
				tkaFile->Load(key.c_str());
				resource.m_tkaFile = std::move(tkaFile);
				return true;
			}
		};


		// ---------------------------------------------------------------
		// TksResource / TksLoader
		// ワーカースレッド: .tks をCPUメモリにロードする。
		// AutoFinalize（メインスレッド）: エンジンバンクに登録し、
		// ModelRender::InitSkeleton() がファイルI/Oをスキップできるようにする。
		// ---------------------------------------------------------------
		class TksResource : public IResource
		{
			friend class TksLoader;

		public:
			void AutoFinalize() override
			{
				if (m_tksFile)
				{
					g_engine->RegistTksFileToBank(m_filePath.c_str(), m_tksFile.get());
					m_tksFile.release();
				}
			}

		private:
			std::string m_filePath;
			std::unique_ptr<nsK2EngineLow::TksFile> m_tksFile;
		};

		class TksLoader : public ResourceLoader<TksResource>
		{
		public:
			bool LoadImpl(TksResource& resource, const std::string& key) override
			{
				resource.m_filePath = key;
				if (g_engine->GetTksFileFromBank(key.c_str()) != nullptr)
				{
					return true;
				}
				auto tksFile = std::make_unique<nsK2EngineLow::TksFile>();
				if (!tksFile->Load(key.c_str()))
				{
					return false;
				}
				resource.m_tksFile = std::move(tksFile);
				return true;
			}
		};

		// ---------------------------------------------------------------
		// DdsWarmResource / DdsWarmLoader
		// ワーカースレッド: DDSバイトを一時バッファに読み込み、
		// OSページキャッシュをウォームアップすることで、
		// メインスレッドのTexture::InitFromDDSFile()がディスクではなくRAMから読める。
		// AutoFinalize は何もしない — バンク登録は不要。
		// ---------------------------------------------------------------
		class DdsWarmResource : public IResource
		{
			friend class DdsWarmLoader;
		public:
			void AutoFinalize() override {}
		};

		class DdsWarmLoader : public ResourceLoader<DdsWarmResource>
		{
		public:
			bool LoadImpl(DdsWarmResource& /*resource*/, const std::string& key) override
			{
				FILE* f = nullptr;
				if (fopen_s(&f, key.c_str(), "rb") != 0 || !f)
					return true; // ファイルが存在しない場合は無視（非致命的）
				fseek(f, 0L, SEEK_END);
				const long size = ftell(f);
				fseek(f, 0L, SEEK_SET);
				if (size > 0)
				{
					std::vector<uint8_t> buf(static_cast<size_t>(size));
					fread(buf.data(), 1, static_cast<size_t>(size), f);
				}
				fclose(f);
				return true;
			}
		};

	} // namespace resource
} // namespace app
