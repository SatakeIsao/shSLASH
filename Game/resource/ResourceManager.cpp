#include "stdafx.h"
#include "ResourceManager.h"

namespace app
{
	namespace resource
	{
		void ResourceManager::Start()
		{
			if (running_.load()) return;
			running_            = true;
			shutdownRequested_  = false;
			worker_ = std::thread(&ResourceManager::WorkerLoop, this);
		}

		void ResourceManager::Shutdown()
		{
			if (!running_.load()) return;
			shutdownRequested_ = true;
			queueCV_.notify_one();
			if (worker_.joinable())
			{
				worker_.join();
			}
			running_ = false;
		}

		void ResourceManager::FinalizeCompleted()
		{
			// AutoFinalize 呼び出し中（エンジンAPIを呼ぶ可能性あり）にロックを保持しないよう、
			// 完了済みだが未フィナライズのリソースを先に収集する。
			std::vector<std::shared_ptr<IResource>> toFinalize;
			{
				std::lock_guard<std::mutex> lk(cacheMutex_);
				for (auto& pair : cache_)
				{
					auto& res = pair.second;
					if (res->IsCompleted() && !res->finalized_.load())
					{
						toFinalize.push_back(res);
					}
				}
			}
			for (auto& res : toFinalize)
			{
				res->AutoFinalize();
				res->finalized_.store(true);
			}
		}


		void ResourceManager::WorkerLoop()
		{
			while (true)
			{
				LoadRequest_ req;

				{
					std::unique_lock<std::mutex> lk(queueMutex_);
					queueCV_.wait(lk, [this]
					{
						return !queue_.empty() || shutdownRequested_.load();
					});

					if (shutdownRequested_.load() && queue_.empty()) break;

					req = std::move(queue_.front());
					queue_.pop();
				}

				req.resource->SetState(IResource::State::Loading);

				bool success = false;
				try
				{
					success = req.loader->DoLoad(*req.resource, req.key);
				}
				catch (...)
				{
					success = false;
				}

				req.resource->SetState(success ? IResource::State::Completed : IResource::State::Error);
				pendingCount_.fetch_sub(1);
			}
		}

	} // namespace resource
} // namespace app
