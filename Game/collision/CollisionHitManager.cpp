#include "stdafx.h"
#include "CollisionHitManager.h"
#include "actor/ActorState.h"
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
			/** DEBUG: ワイヤーフレーム描画 */
			//PhysicsWorld::Get().EnableDrawDebugWireFrame();
		}


		CollisionHitManager::~CollisionHitManager()
		{

		}


		void CollisionHitManager::Update()
		{
			//デバッグ用現在のヒットペアの数を出力
			char countBuf[256];
			sprintf_s(countBuf, "--- Hit Pair Count: %zu ---\n", hitPairList_.size());
			OutputDebugStringA(countBuf);

			// GhostBodyのヒット処理で得たペアをもとに処理
			{
				app::memory::StackAllocatorMarker marker;
				app::memory::StackVector<Pair*>  eventCharacterPairList(marker);
				for (auto& hitPair : hitPairList_) {
					char idBuf[256];
					sprintf_s(idBuf, "Collision! A_ID: %u, B_ID: %u\n", hitPair.a->GetOwnerId(), hitPair.b->GetOwnerId());
					OutputDebugStringA(idBuf);
					// イベントキャラクターのペアか
					if (ContainsEventCharacterPair(hitPair)) {
						eventCharacterPairList.push_back(&hitPair);
					}
				}
				for (auto* pair : eventCharacterPairList) {
					UpdateEventCharacterPair(*pair);
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
			if (!IsHitObject<app::actor::EventCharacter>(hitPair)) {
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
			auto* eventCharacter = GetHitObject <app::actor::EventCharacter>(hitPair);

			Vector3 playerPos = battleCharacter->transform.position;
			Vector3 slimePos = eventCharacter->transform.position;

			//パンチされたかのチェック
			app::collision::GhostBody* colliedPlayerBody = nullptr;
			if (hitPair.a->GetOwnerId() == app::actor::BattleCharacter::ID())
			{
				colliedPlayerBody = hitPair.a;
			}
			else if (hitPair.b->GetOwnerId() == app::actor::BattleCharacter::ID())
			{
				colliedPlayerBody = hitPair.b;
			}

			//パンチの当たり判定の時
			if (colliedPlayerBody != nullptr
				&& colliedPlayerBody != battleCharacter->GetGhostBody())
			{
				//プレイヤーからスライムへのベクトルを計算
				Vector3 knockBackDirection = slimePos - playerPos;
				knockBackDirection.y = 0.0f;
				knockBackDirection.Normalize();
				//スライムがノックバックした
				eventCharacter->GetStateMachine()->OnKnockBack(knockBackDirection);
			}
			/** プレイヤー本体のゴーストと衝突した時 */
			else
			{
				/** スライムからプレイヤーに向かうベクトル */
				Vector3 toPlayer = playerPos - slimePos;
				toPlayer.Normalize();
				float dot = toPlayer.y;

				bool isAbove = (dot > 0.1f);

				// Playerが上空にいるなら
				if (isAbove)
				{
					eventCharacter->GetStateMachine()->OnSquashed();
				}
				else
				{
					battleCharacter->GetStateMachine()->OnKnockBack();
				}
			}
		}
	}
}