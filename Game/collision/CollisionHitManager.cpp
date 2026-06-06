#include "stdafx.h"
#include "CollisionHitManager.h"
#include "actor/ActorState.h"
#include "actor/ActorStateMachine.h"
#include "actor/BattleCharacter.h"
#include "actor/EventCharacter.h"
#include "actor/Gimmick.h"
#include "battle/BattleManager.h"
#include "util/ParallelFor.h"

namespace
{
	template <typename T>
	bool IsHitObject(const app::collision::CollisionHitManager::Pair& pair)
	{
		if (pair.a->GetOwnerId() == T::ID()) {
			return true;
		}
		if (pair.b->GetOwnerId() == T::ID()) {
			return true;
		}
		return false;
	}

	template <typename T>
	T* GetHitObject(app::collision::CollisionHitManager::Pair& pair)
	{
		if (pair.a->GetOwnerId() == T::ID()) {
			return static_cast<T*>(pair.a->GetOwner());
		}
		if (pair.b->GetOwnerId() == T::ID()) {
			return static_cast<T*>(pair.b->GetOwner());
		}
		return nullptr;
	}
}


namespace app
{
	namespace collision
	{
		CollisionHitManager* CollisionHitManager::instance_ = nullptr;


		CollisionHitManager::CollisionHitManager()
		{
			/** DEBUG: 物理デバッグワイヤーフレーム */
			//PhysicsWorld::Get().EnableDrawDebugWireFrame();
		}


		CollisionHitManager::~CollisionHitManager()
		{

		}


		void CollisionHitManager::Update()
		{
			// GhostBodyのヒット情報を一旦ペアごとに処理
			{
				app::memory::StackAllocatorMarker marker;
				app::memory::StackVector<Pair*> eventCharacterPairList(marker);
				app::memory::StackVector<Pair*> mushroomPairList(marker);
				for (auto& hitPair : hitPairList_) {
					if (ContainsEventCharacterPair(hitPair)) {
						eventCharacterPairList.push_back(&hitPair);
					}
					else if (ContainsMushroomEventCharacterPair(hitPair)) {
						mushroomPairList.push_back(&hitPair);
					}
				}
				for (auto* pair : eventCharacterPairList) {
					UpdateEventCharacterPair(*pair);
				}
				for (auto* pair : mushroomPairList) {
					UpdateMushroomEventCharacterPair(*pair);
				}
			}
			hitPairList_.clear();
		}


		void CollisionHitManager::RegisterHitPair(app::collision::GhostBody* a, app::collision::GhostBody* b)
		{
			// ヒットペア登録
			hitPairList_.push_back(std::move(Pair(a, b)));
		}


		bool CollisionHitManager::ContainsEventCharacterPair(const Pair& hitPair)
		{
			if (!IsHitObject<app::actor::StoneEventCharacter>(hitPair)) {
				return false;
			}
			if (!IsHitObject<app::actor::BattleCharacter>(hitPair)) {
				return false;
			}
			return true;
		}


		void CollisionHitManager::UpdateEventCharacterPair(Pair& hitPair)
		{
			auto* battleCharacter = GetHitObject<app::actor::BattleCharacter>(hitPair);
			auto* eventCharacter = GetHitObject <app::actor::StoneEventCharacter>(hitPair);

			Vector3 playerPos = battleCharacter->transform.position;
			Vector3 enemyPos = eventCharacter->transform.position;

			// ノックバックベクトルの計算
			Vector3 knockBackDirection = enemyPos - playerPos;
			knockBackDirection.y = 0.0f;
			knockBackDirection.Normalize();

			// パーツ（判定用ボディ）で当たったかのチェック
			app::collision::GhostBody* colliedPlayerBody = nullptr;
			if (hitPair.a->GetOwnerId() == app::actor::BattleCharacter::ID())
			{
				colliedPlayerBody = hitPair.a;
			}
			else if (hitPair.b->GetOwnerId() == app::actor::BattleCharacter::ID())
			{
				colliedPlayerBody = hitPair.b;
			}

			// 敵側のゴーストボディを取得（本体か攻撃ゴーストかを識別するため）
			app::collision::GhostBody* colliedEnemyBody = nullptr;
			if (hitPair.a->GetOwnerId() == app::actor::StoneEventCharacter::ID())
			{
				colliedEnemyBody = hitPair.a;
			}
			else if (hitPair.b->GetOwnerId() == app::actor::StoneEventCharacter::ID())
			{
				colliedEnemyBody = hitPair.b;
			}

			// パーツの判定処理
			if (colliedPlayerBody != nullptr
				&& colliedPlayerBody != battleCharacter->GetGhostBody())
			{
				// 溜め攻撃中かどうか判定
				bool isBlowBack = battleCharacter->GetStateMachine()->IsChargeAttacking();
				int chargeLevel = battleCharacter->GetStateMachine()->GetChargeLevel();
				int comboIndex = battleCharacter->GetStateMachine()->isSlashSecond() ? 1 : 0;
				// 攻撃パーツの衝突を通知
				auto* notify = new app::battle::BattleManager::DamageNotify();
				notify->attacker = battleCharacter;
				notify->defender = eventCharacter;
				notify->knockBackDirection = knockBackDirection;
				notify->enemyType = app::battle::BattleManager::DamageNotify::EnemyType::Stone;
				notify->isBlowBack = isBlowBack;
				notify->chargeLevel = chargeLevel;
				notify->comboIndex = comboIndex;
				app::battle::BattleManager::Get().AddNotify(notify);

				//// プレイヤーからエネミーへのベクトルを計算
				//Vector3 knockBackDirection = enemyPos - playerPos;
				//knockBackDirection.y = 0.0f;
				//knockBackDirection.Normalize();
				//// エネミーのノックバック
				//eventCharacter->GetStateMachine()->OnKnockBack(knockBackDirection);
				//
				//float attack = battleCharacter->GetTotalAttack();
				//eventCharacter->TakeDamage(static_cast<int>(attack));
				//// HPが0になったらDeadステートへ
				//if (eventCharacter->GetCurrentHP() <= 0)
				//{
				//	eventCharacter->GetStateMachine()->OnDead();
				//}
			}
			/** プレイヤー本体のゴースト（実体）と衝突した場合 */
			else
			{
				/** 回避中はノックバックを受けない */
				if (battleCharacter->GetStateMachine()->IsAvoiding()) return;

				/** エネミーからプレイヤーに向かうベクトル */
				Vector3 toPlayer = playerPos - enemyPos;
				toPlayer.Normalize();
				float dot = toPlayer.y;
				bool isAbove = (dot > 0.1f);

				// Playerが上に乗ったなら
				if (isAbove)
				{
					eventCharacter->GetStateMachine()->OnSquashed();
				}
				else
				{
					battleCharacter->GetStateMachine()->OnKnockBack();
					// 攻撃ゴーストが実際に当たった時だけHPダメージを通知
					if (colliedEnemyBody != nullptr && colliedEnemyBody != eventCharacter->GetGhostBody())
					{
						eventCharacter->GetStateMachine()->NontifyAttackGhostCreated();
					}
				}
			}
		}


		bool CollisionHitManager::ContainsMushroomEventCharacterPair(const Pair& hitPair)
		{
			if (!IsHitObject<app::actor::MushroomEventCharacter>(hitPair)) {
				return false;
			}
			if (!IsHitObject<app::actor::BattleCharacter>(hitPair)) {
				return false;
			}
			return true;
		}


		void CollisionHitManager::UpdateMushroomEventCharacterPair(Pair& hitPair)
		{
			auto* battleCharacter = GetHitObject<app::actor::BattleCharacter>(hitPair);
			auto* eventCharacter = GetHitObject <app::actor::MushroomEventCharacter>(hitPair);

			Vector3 playerPos = battleCharacter->transform.position;
			Vector3 enemyPos = eventCharacter->transform.position;

			// ノックバックベクトルの計算
			Vector3 knockBackDirection = enemyPos - playerPos;
			knockBackDirection.y = 0.0f;
			knockBackDirection.Normalize();

			// パーツ（判定用ボディ）で当たったかのチェック
			app::collision::GhostBody* colliedPlayerBody = nullptr;
			if (hitPair.a->GetOwnerId() == app::actor::BattleCharacter::ID())
			{
				colliedPlayerBody = hitPair.a;
			}
			else if (hitPair.b->GetOwnerId() == app::actor::BattleCharacter::ID())
			{
				colliedPlayerBody = hitPair.b;
			}

			// 敵側のゴーストボディを取得（本体か攻撃ゴーストかを識別するため）
			app::collision::GhostBody* colliedEnemyBody = nullptr;
			if (hitPair.a->GetOwnerId() == app::actor::MushroomEventCharacter::ID())
			{
				colliedEnemyBody = hitPair.a;
			}
			else if (hitPair.b->GetOwnerId() == app::actor::MushroomEventCharacter::ID())
			{
				colliedEnemyBody = hitPair.b;
			}

			// パーツの判定処理
			if (colliedPlayerBody != nullptr
				&& colliedPlayerBody != battleCharacter->GetGhostBody())
			{
				// 溜め攻撃中かどうか判定
				bool isBlowBack = battleCharacter->GetStateMachine()->IsChargeAttacking();
				int chargeLevel = battleCharacter->GetStateMachine()->GetChargeLevel();
				int comboIndex = battleCharacter->GetStateMachine()->isSlashSecond() ? 1 : 0;
				// 攻撃パーツの衝突を通知
				auto* notify = new app::battle::BattleManager::DamageNotify();
				notify->attacker = battleCharacter;
				notify->defender = eventCharacter;
				notify->knockBackDirection = knockBackDirection;
				notify->enemyType = app::battle::BattleManager::DamageNotify::EnemyType::Mushroom;
				notify->isBlowBack = isBlowBack;
				notify->chargeLevel = chargeLevel;
				notify->comboIndex = comboIndex;
				app::battle::BattleManager::Get().AddNotify(notify);
			}
			/** プレイヤー本体のゴースト（実体）と衝突した場合 */
			else
			{
				/** 回避中はノックバックを受けない */
				if (battleCharacter->GetStateMachine()->IsAvoiding()) return;

				/** エネミーからプレイヤーに向かうベクトル */
				Vector3 toPlayer = playerPos - enemyPos;
				toPlayer.Normalize();
				float dot = toPlayer.y;
				bool isAbove = (dot > 0.1f);

				// Playerが上に乗ったなら
				if (isAbove)
				{
					eventCharacter->GetStateMachine()->OnSquashed();
				}
				else
				{
					battleCharacter->GetStateMachine()->OnKnockBack();
					// 攻撃ゴーストが実際に当たった時だけHPダメージを通知
					if (colliedEnemyBody != nullptr && colliedEnemyBody != eventCharacter->GetGhostBody())
					{
						eventCharacter->GetStateMachine()->NontifyAttackGhostCreated();
					}
				}
			}
		}
	}
}