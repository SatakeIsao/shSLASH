#include "stdafx.h"
#include "actor/EnemyAttackPointManager.h"


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


		void EnemyAttackPointManager::RequestAttackToken(Character* enemy)
		{
			for (Character* c : attackQueue_)
			{
				if (c == enemy) { return; }
			}

			attackQueue_.push_back(enemy);
		}


		bool EnemyAttackPointManager::ConsumeAttackToken(Character* enemy)
		{
			if (attackQueue_.empty()) { return false; }
			if (attackQueue_.front() != enemy) { return false; }

			attackQueue_.pop_front();
			return true;
		}


		void EnemyAttackPointManager::RemoveFromToken(Character* enemy)
		{
			attackQueue_.erase(std::remove(attackQueue_.begin(), attackQueue_.end(), enemy), attackQueue_.end());
		}
	}
}