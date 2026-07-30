#pragma once
#include "actor/EnemyAttackPoint.h"
#include <deque>


namespace app
{
	namespace actor
	{
		class Character;

		class EnemyAttackPointManager
		{
		private:
			EnemyAttackPoint attackPoint_;
			std::deque<Character*> attackQueue_;	// 待機ポイントのキュー


		public:
			EnemyAttackPointManager() = default;
			~EnemyAttackPointManager() = default;


			/** 更新処理 */
			void Update(Vector3 position);

			/** 攻撃ポイントの取得し、使用中にする */
			EnemyAttackPoint::AttackPoint* AcquireAttackPoint(Vector3 position, Character* enemy);


			/** より近い攻撃ポイントの再取得し、使用中にする */
			EnemyAttackPoint::AttackPoint* ReAcquireAttackPoint(Character* enemy, EnemyAttackPoint::AttackPoint* attackPoint);


			/** 攻撃ポイントの解放 */
			void ReleaseAttackPoint(EnemyAttackPoint::AttackPoint* attackPoint, Character* enemy);


			/** 使用可能かどうか */
			bool IsUseable() const;


			EnemyAttackPoint::AttackPoint* GetNearWaitPoint(Vector3 position) { return attackPoint_.GetNearWaitPoint(position); }


			/** 空いている待機ポイントを予約する（1点=1体を保証する） */
			EnemyAttackPoint::AttackPoint* AcquireWaitPoint(Vector3 position, Character* enemy);
			/** 待機ポイントの解放 */
			void ReleaseWaitPoint(EnemyAttackPoint::AttackPoint* waitPoint, Character* enemy);


			/** 攻撃トークンの要求 */
			void RequestAttackToken(Character* enemy);


			/** キューの先頭が自分なら攻撃権を消費してtrueを返す */
			bool ConsumeAttackToken(Character* enemy);


			/** 攻撃トークンの解放 */
			void RemoveFromToken(Character* enemy);
			
		};
	}
}

