#pragma once
#include <array>

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
			};


		private:
			static constexpr int kAttackPointNum = 15;					// 攻撃ポイントの数
			static constexpr int kAttackPointUseLimit = 1;				// 同時使用数の上限
			static constexpr float kToAttackPointDistance = 200.0f;		// 攻撃ポイントまでの距離の閾値


		private:
			std::array<AttackPoint, kAttackPointNum> attackPointList_;	// 攻撃ポイントの配列
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


			static constexpr int GetAttackPointNum() { return kAttackPointNum; }
		};
	}
}