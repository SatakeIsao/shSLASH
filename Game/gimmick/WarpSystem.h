/**
 * WarpSystem.h
 * 土管でワープのような処理を行う処理群
 */
#pragma once


namespace app
{
	namespace actor { class PipeGimmick; }


	namespace gimmick
	{
		using EndpointId = uint32_t;


		// 土管の接続情報
		struct WarpLink
		{
			EndpointId fromId = -1;
			EndpointId toId = -1;
		};


		class WarpSystem
		{
		private:
			std::unordered_map<EndpointId, app::actor::PipeGimmick*> registMap_;
			std::unordered_map<EndpointId, EndpointId> linkMap_;


		private:
			WarpSystem();
			~WarpSystem();


		public:
			/** Endpoint管理 */
			void RegisterPipe(EndpointId id, app::actor::PipeGimmick* pipe);
			void UnregisterPipe(EndpointId id);
			const app::actor::PipeGimmick* FindPipe(EndpointId id) const;

			/**
			 * Router (接続解決)
			 */
			/** リンク情報の登録（レベルロード時などに呼ぶ） */
			void AddLink(EndpointId from, EndpointId to);
			/** 行き先解決 */
			bool TryResolve(EndpointId fromId, EndpointId& outToId) const;




			/**
			 * シングルトン関連
			 */
		private:
			static WarpSystem* instance_;


		public:
			static void Initialize()
			{
				if (instance_ == nullptr)
				{
					instance_ = new WarpSystem();
				}
			}
			static WarpSystem& Get() { return *instance_; }
			static bool IsAvailable() { return instance_ != nullptr; }
			static void Finalize()
			{
				if (instance_) {
					delete instance_;
					instance_ = nullptr;
				}
			}
		};
	}
}