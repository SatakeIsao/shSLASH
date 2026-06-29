#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <memory>
#include <string>
#include <atomic>
#include <typeindex>
#include <cstdio>
#include <vector>

namespace app
{
	namespace resource
	{
		// 状態追跡と自動フィナライズフックを持つリソース基底クラス。
		class IResource
		{
		public:
			enum class State { None, Loading, Completed, Error };

			virtual ~IResource() = default;

			State GetState() const
			{
				std::lock_guard<std::mutex> lk(mutex_);
				return state_;
			}
			bool IsCompleted() const { return GetState() == State::Completed; }
			bool IsError()     const { return GetState() == State::Error; }
			bool IsFinalized() const { return finalized_.load(); }

			// ワーカーが状態をCompletedに設定した後、メインスレッドで一度だけ呼ばれる。
			// オーバーライドしてロード済みデータをエンジンバンクに登録する。
			virtual void AutoFinalize() {}

		protected:
			IResource() = default;

		private:
			friend class ResourceManager;

			void SetState(State s)
			{
				std::lock_guard<std::mutex> lk(mutex_);
				state_ = s;
			}

			State              state_     = State::None;
			std::atomic<bool>  finalized_ { false };
			mutable std::mutex mutex_;
		};


		// ローダー基底インターフェース
		class IResourceLoader
		{
		public:
			virtual ~IResourceLoader() = default;
			virtual bool DoLoad(IResource& resource, const std::string& key) = 0;
			virtual std::shared_ptr<IResource> CreateResource() = 0;
		};

		template <typename T>
		class ResourceLoader : public IResourceLoader
		{
		public:
			virtual bool LoadImpl(T& resource, const std::string& key) = 0;

			bool DoLoad(IResource& resource, const std::string& key) override
			{
				return LoadImpl(static_cast<T&>(resource), key);
			}
			std::shared_ptr<IResource> CreateResource() override
			{
				return std::make_shared<T>();
			}
		};


		// ワーカースレッドに渡す内部ロードリクエスト
		struct LoadRequest_
		{
			std::shared_ptr<IResourceLoader> loader;
			std::string                      key;
			std::shared_ptr<IResource>       resource;
		};


		// 非同期リソースマネージャー（シングルトン、ワーカースレッド1本）。
		//
		// 使い方:
		//   1. Register<T>(loader) を起動時に一度呼ぶ
		//   2. rm.Load<T>(path)    -- 即時返却、処理はキューに積まれる
		//   3. rm.FinalizeCompleted() -- 毎フレーム（メインスレッド）呼び出し、完了したリソースをバンク登録する
		class ResourceManager
		{
		public:
			ResourceManager() = default;
			~ResourceManager() { Shutdown(); }

			ResourceManager(const ResourceManager&) = delete;
			ResourceManager& operator=(const ResourceManager&) = delete;

			// Start() を呼ぶ前に型付きローダーを登録する。
			template <typename T>
			void Register(std::shared_ptr<ResourceLoader<T>> loader)
			{
				std::lock_guard<std::mutex> lk(loaderMutex_);
				loaders_[std::type_index(typeid(T))] = std::move(loader);
			}

			// 非同期ロードをリクエストする。共有ハンドルを即時返却する。
			// リソースはFinalizeCompleted()によって自動的にバンク登録される。
			template <typename T>
			std::shared_ptr<T> Load(const std::string& key);

			// メインスレッドでフレームごとに一度呼ぶ（シーンUpdateの前）。
			// 新たに完了したリソース全てにAutoFinalize()を呼び出す。
			void FinalizeCompleted();

			void Start();
			void Shutdown();

			bool IsIdle() const { return pendingCount_.load() == 0; }

		private:
			void WorkerLoop();

			struct CacheKey
			{
				std::type_index type;
				std::string     name;
				bool operator==(const CacheKey& o) const
				{
					return type == o.type && name == o.name;
				}
			};
			struct CacheKeyHash
			{
				std::size_t operator()(const CacheKey& k) const
				{
					auto h1 = k.type.hash_code();
					auto h2 = std::hash<std::string>{}(k.name);
					return h1 ^ (h2 * 2654435761u);
				}
			};

			mutable std::mutex cacheMutex_;
			std::unordered_map<CacheKey, std::shared_ptr<IResource>, CacheKeyHash> cache_;

			mutable std::mutex loaderMutex_;
			std::unordered_map<std::type_index, std::shared_ptr<IResourceLoader>> loaders_;

			std::mutex               queueMutex_;
			std::condition_variable  queueCV_;
			std::queue<LoadRequest_> queue_;

			std::thread       worker_;
			std::atomic<bool> running_{ false };
			std::atomic<bool> shutdownRequested_{ false };
			std::atomic<int>  pendingCount_{ 0 };

		public:
			static ResourceManager& GetInstance()
			{
				static ResourceManager instance;
				return instance;
			}
		};


		// Load<T> テンプレート実装（ヘッダーに必要）
		template <typename T>
		std::shared_ptr<T> ResourceManager::Load(const std::string& key)
		{
			CacheKey ck{ std::type_index(typeid(T)), key };

			// キャッシュチェック
			{
				std::lock_guard<std::mutex> lk(cacheMutex_);
				auto it = cache_.find(ck);
				if (it != cache_.end())
				{
					return std::static_pointer_cast<T>(it->second);
				}
			}

			// ローダー検索
			std::shared_ptr<IResourceLoader> loader;
			{
				std::lock_guard<std::mutex> lk(loaderMutex_);
				auto it = loaders_.find(std::type_index(typeid(T)));
				if (it != loaders_.end())
				{
					loader = it->second;
				}
			}

			if (!loader)
			{
				std::fprintf(stderr, "[ResourceManager] no loader registered for type\n");
				return nullptr;
			}

			// リソースを生成してキャッシュに登録
			auto resource = loader->CreateResource();
			auto typed    = std::static_pointer_cast<T>(resource);

			{
				std::lock_guard<std::mutex> lk(cacheMutex_);
				auto result = cache_.emplace(ck, resource);
				if (!result.second)
				{
					// 別スレッドが先に登録済み。そのエントリを使用
					return std::static_pointer_cast<T>(result.first->second);
				}
			}

			// ワーカースレッドにキューイング
			{
				std::lock_guard<std::mutex> lk(queueMutex_);
				LoadRequest_ req;
				req.loader   = loader;
				req.key      = key;
				req.resource = resource;
				queue_.push(std::move(req));
			}

			pendingCount_.fetch_add(1);
			queueCV_.notify_one();
			return typed;
		}

	} // namespace resource
} // namespace app
