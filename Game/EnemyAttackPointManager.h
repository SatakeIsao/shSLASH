#pragma once
#include "EnemyAttackPoint.h"


namespace app
{
	namespace actor
	{
		class Character;

		class EnemyAttackPointManager
		{
		private:
			EnemyAttackPoint attackPoint_;


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
		};
	}
}

