#include "stdafx.h"
#include "actor/Actor.h"
#include "EnemyAttackPoint.h"


namespace app
{
	namespace actor
	{
		EnemyAttackPoint::EnemyAttackPoint()
		{
			for (int i = 0; i < kAttackPointNum; ++i)
			{
				attackPointList_[i].number_ = i;
			}
		}

		EnemyAttackPoint::~EnemyAttackPoint()
		{
		}


		void EnemyAttackPoint::Update(Vector3 position)
		{
			Vector3 direction = g_vec3Front;

			Quaternion directionRot = g_quatIdentity;

			directionRot.SetRotationDegY(360.0f / static_cast<float>(kAttackPointNum));

			for(AttackPoint& attackPoint : attackPointList_)
			{
				attackPoint.position_ = position + direction * kToAttackPointDistance;
				directionRot.Apply(direction);
			}
		}


		EnemyAttackPoint::AttackPoint* EnemyAttackPoint::GetNearAttackPoint(Vector3 position)
		{
			if (!IsUsableAttackPoint())
			{
				return nullptr;
			}

			float nearestDistance = FLT_MAX;
			AttackPoint* nearest = nullptr;

			for (AttackPoint& attackPoint : attackPointList_)
			{
				if (attackPoint.use_) continue;

				const float distance = (attackPoint.position_ - position).Length();
				if (distance < nearestDistance)
				{
					nearestDistance = distance;
					nearest = &attackPoint;
				}
			}

			return nearest;
		}


		EnemyAttackPoint::AttackPoint* EnemyAttackPoint::GetNearAttackPoint(Vector3 position, Character* enemy)
		{
			if (!IsUsableAttackPoint())
			{
				return nullptr;
			}

			float nearestDistance = FLT_MAX;
			AttackPoint* nearest = nullptr;

			for (AttackPoint& attackPoint : attackPointList_)
			{
				if (attackPoint.use_) continue;	

				const float distance = (attackPoint.position_ - position).Length();
				if (distance < nearestDistance)
				{
					nearestDistance = distance;
					nearest = &attackPoint;
				}
			}

			if (nearest != nullptr)
			{
				UseAttackPoint(nearest->number_, enemy);
			}

			return nearest;
		}


		EnemyAttackPoint::AttackPoint* EnemyAttackPoint::ReGetNearAttackPoint(Character* enemy, AttackPoint* attackPoint)
		{
			// 渡されたポイントが対象のエネミーのものでなければ何もしない
			if (attackPoint == nullptr || attackPoint->useEnemy_ != enemy)
			{
				return nullptr;
			}

			// 現在のポイントを解放する
			ReleaseAttackPoint(attackPoint->number_, enemy);

			float nearestDistance = FLT_MAX;
			AttackPoint* nearest = nullptr;

			for (AttackPoint& ap : attackPointList_)
			{
				// 使用中のポイントはスキップ
				if (ap.use_) { continue; }

				const float distance = (ap.position_ - enemy->transform.position).Length();
				if (distance < nearestDistance)
				{
					nearestDistance = distance;
					nearest = &ap;
				}
			}

			// 最近傍ポイントを使用中にする
			if (nearest != nullptr)
			{
				UseAttackPoint(nearest->number_, enemy);
			}

			return nearest;
		}


		bool EnemyAttackPoint::IsUsableAttackPoint() const
		{
			return useAttackPointNum_ < kAttackPointUseLimit;
		}


		void EnemyAttackPoint::UseAttackPoint(int number, Character* enemy)
		{
			// すでに使用中なら何もしない
			if (attackPointList_[number].use_) { return; }

			attackPointList_[number].use_ = true;
			attackPointList_[number].useEnemy_ = enemy;
			useAttackPointNum_++;
		}


		void EnemyAttackPoint::ReleaseAttackPoint(int number, Character* enemy)
		{
			// ポイントを確保しているエネミーが異なれば何もしない
			if (attackPointList_[number].useEnemy_ != enemy) { return; }

			attackPointList_[number].use_ = false;
			attackPointList_[number].useEnemy_ = nullptr;
			useAttackPointNum_--;
		}
	}
}