#include "stdafx.h"
#include "EnemyAttackPointManager.h"


namespace app
{
	namespace actor
	{
		void EnemyAttackPointManager::Update(Vector3 position)
		{
			attackPoint_.Update(position);
		}


		EnemyAttackPoint::AttackPoint* EnemyAttackPointManager::AcquireAttackPoint(Vector3 position, Character* enemy)
		{
			return attackPoint_.GetNearAttackPoint(position, enemy);
		}


		EnemyAttackPoint::AttackPoint* EnemyAttackPointManager::ReAcquireAttackPoint(Character* enemy, EnemyAttackPoint::AttackPoint* attackPoint)
		{
			return attackPoint_.ReGetNearAttackPoint(enemy, attackPoint);
		}


		void EnemyAttackPointManager::ReleaseAttackPoint(EnemyAttackPoint::AttackPoint* attackPoint, Character* enemy)
		{
			if (attackPoint == nullptr) { return; }
			attackPoint_.ReleaseAttackPoint(attackPoint->number_, enemy);
		}

		bool EnemyAttackPointManager::IsUseable() const
		{
			return attackPoint_.IsUsableAttackPoint();
		}
	}
}