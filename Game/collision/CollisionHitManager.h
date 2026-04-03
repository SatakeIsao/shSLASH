/**
 * CollisionHitManager.h
 * 衝突ヒット管理
 */
#pragma once


namespace app
{
	namespace collision
	{
		class GhostBody;


		/**
		 * 衝突ヒット管理クラス
		 */
		class CollisionHitManager
		{
		private:
			struct Pair
			{
				app::collision::GhostBody* a = nullptr;
				app::collision::GhostBody* b = nullptr;
				//
				Pair(app::collision::GhostBody* bodyA, app::collision::GhostBody* bodyB)
					: a(bodyA), b(bodyB)
				{
				}
			};


		private:
			std::vector<Pair> hitPairList_;
			

		private:
			CollisionHitManager();
			~CollisionHitManager();


		public:
			/** 更新 */
			void Update();

			/** 衝突ペア登録 */
			void RegisterHitPair(app::collision::GhostBody* a, app::collision::GhostBody* b);


		private:
			/** スライムを含むペアか */
			bool ContainsEventCharacterPair(const Pair& hitPair);
			void UpdateEventCharacterPair(Pair& hitPair);



			/**
			 * シングルトン関連
			 */
		private:
			static CollisionHitManager* instance_;


		public:
			static void Initialize()
			{
				if (!instance_) {
					instance_ = new CollisionHitManager();
				}
			}
			static CollisionHitManager& Get() { return *instance_; }
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