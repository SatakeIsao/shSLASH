/**
 * ActorStatusファイル
 * ステータスをまとめたクラス群
 */
#pragma once


namespace app
{
	namespace actor
	{
		/**
		 * キャラクターステータス基底構造体
		 */
		struct IActorStatus
		{
		public:
			virtual ~IActorStatus() {}

			/** パラメーターファイル読み込み */
			virtual void LoadParameter(const char* path) = 0;
			/** パラメーター数値設定等を行う関数 */
			virtual void Setup() = 0;


		public:
			template <typename T>
			inline bool Is() const
			{
				auto* ptr = dynamic_cast<const T*>(this);
				return ptr != nullptr;
			}
			template <typename T>
			inline T* As()
			{
				return dynamic_cast<T*>(this);
			}
		};




		/** キャラクター用 */
		struct CharacterStatus : public IActorStatus
		{
		protected:
			float moveSpeed_ = 1.0f;
			float jumpMoveSpeed_ = 1.0f;
			float jumpPower_ = 1.0f;
			float friction_ = 1.0f;
			float gravity_ = 0.98f;
			float radius_ = 0.0f;
			float height_ = 0.0f;

			float warpStartScale_ = 1.0f;
			float warpEndScale_ = 0.3f;
			float warpTimeSeconds_ = 1.0f;


		public:
			virtual ~CharacterStatus() {}

			virtual void LoadParameter(const char* path) override {}
			virtual void Setup() override {}


		public:
			inline float GetMoveSpeed() const { return moveSpeed_; }
			inline float GetJumpMoveSpeed() const { return jumpMoveSpeed_; }
			inline float GetJumpPower() const { return jumpPower_; }
			inline float GetRadius() const { return radius_; }
			inline float GetHeight() const { return height_; }

			inline void SetRadius(const float radius) { radius_ = radius; }
			inline void SetHeight(const float height) { height_ = height; }

			inline float GetFriction() const { return friction_; }
			inline void SetFriction(const float friction) { friction_ = friction; }

			inline float GetGravity() const { return gravity_; }
			inline void SetGravity(const float gravity) { gravity_ = gravity; }


			inline float GetWarpStartScale() const { return warpStartScale_; }
			inline float GetWarpEndScale() const { return warpEndScale_; }
			inline float GetWarpTimeSeconds() const { return warpTimeSeconds_; }
			inline void SetWarpData(const float startScale, const float endScale, const float timeSeconds)
			{
				warpStartScale_ = startScale;
				warpEndScale_ = endScale;
				warpTimeSeconds_ = timeSeconds;
			}
		};




		/** バトルキャラクター用 */
		struct BattleCharacterStatus : public CharacterStatus
		{
		protected:


		public:
			virtual ~BattleCharacterStatus() {}
			virtual void LoadParameter(const char* path) override;
			virtual void Setup() override;
		};




		/** イベントキャラクター用 */
		struct EventCharacterStatus : public CharacterStatus
		{
		protected:


		public:
			virtual ~EventCharacterStatus() {}
			virtual void LoadParameter(const char* path) override;
			virtual void Setup() override;
		};
	}
}
