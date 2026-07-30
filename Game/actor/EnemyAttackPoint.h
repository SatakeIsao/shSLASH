#pragma once
#include <array>
#include <vector>

namespace app
{
	namespace actor
	{
		class Character;

		class EnemyAttackPoint
		{
		public:
			struct AttackPoint
			{
				Vector3 position_;				// 攻撃ポイントの位置
				int number_ = 0;				// 攻撃ポイントの番号
				bool use_ = false;				// 攻撃ポイントが使用されているかどうか
				Character* useEnemy_ = nullptr;	// 攻撃ポイントを使用している敵キャラクター

				float cooldownTimer_ = 0.0f;	// クールダウンタイマー
				bool isOnCooldown_ = false;		// クールダウン中かどうか
			};


		private:
			static constexpr int kAttackPointNum = 13;					// 攻撃ポイントの数
			static constexpr int kAttackPointUseLimit = 2;				// 同時使用数の上限
			static constexpr float kToAttackPointDistance = 50.0f;		// 攻撃ポイントまでの距離の閾値
			static constexpr float kToWaitPointDistance = 75.0f;		// 待機ポイントまでの距離の閾値
			static constexpr float kAttackPointCooldownTime = 3.0f;		// 攻撃ポイントのクールダウン時間


		private:
			std::array<AttackPoint, kAttackPointNum> attackPointList_;	// 攻撃ポイントの配列
			std::array<AttackPoint, kAttackPointNum >waitPointList_;	// 待機ポイントの配列
			int useAttackPointNum_ = 0;									// 現在使用されている攻撃ポイントの数


		public:
			EnemyAttackPoint();
			~EnemyAttackPoint();


			/** アタックポイントの座標を更新 */
			void Update(Vector3 position);


			/** 近くにあるアタックポイントのアドレスを取得する（使用中にはしない）*/
			AttackPoint* GetNearAttackPoint(Vector3 position);


			/** 近くにあるアタックポイントのアドレスを取得する（使用中にする）*/
			AttackPoint* GetNearAttackPoint(Vector3 position, Character* enemy);


			/** より近いアタックポイントを取得しなおす。現在確保しているポイントを解放してから再取得する */
			AttackPoint* ReGetNearAttackPoint(Character* enemy, AttackPoint* attackPoint);


			/** アタックポイントを使用中にする */
			void UseAttackPoint(int number, Character* enemy);


			/** アタックポイントの解放 */
			void ReleaseAttackPoint(int number, Character* enemy);


			/** アタックポイントが使用可能かどうかを判定する */
			bool IsUsableAttackPoint() const;


			/** 近くにある待機ポイントのアドレスを取得する（予約はしない） */
			AttackPoint* GetNearWaitPoint(Vector3 position);


			/** 空いている最近傍の待機ポイントを取得し、使用中にする（1点=1体を保証するための予約） */
			AttackPoint* AcquireWaitPoint(Vector3 position, Character* enemy);


			/** 待機ポイントの解放 */
			void ReleaseWaitPoint(int number, Character* enemy);


			static constexpr int GetAttackPointNum() { return kAttackPointNum; }
		};
	}
}