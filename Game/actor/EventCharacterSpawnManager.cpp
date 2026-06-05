#include "stdafx.h"
#include "EventCharacterSpawnManager.h"
#include "actor/EventCharacter.h"
#include "actor/BattleCharacter.h"
#include "effect/EffectManager.h"
#include "ui/InGameUI.h"
#include <functional>


namespace
{
	//const int MAX_EVENT_CHARACTER = 10;                   // 同時に存在できるイベントキャラクターの最大数
	const int INITIAL_SPAWN_COUNT = 10;                   // 初期スポーン数
	const int MAX_PLAYER_LEVEL = 10;                      // プレイヤーレベルの最大値
	const int SKELETON_SPAWN_LEVEL = 6;                   // スケルトンがスポーンし始めるプレイヤーレベル
	const float SKELETON_BASE_PROBABILITY = 0.1f;         // スケルトンの基本出現確率 (Lv6で10%)
	const float SKELETON_PROBABILITY_INCREMENT = 0.1f;    // プレイヤーレベルが1上がるごとにスケルトンの出現確率が上昇する量
	const float MAX_SKELETON_PROBABILITY = 0.6f;          // スケルトンの最大出現確率 (60%)
	const float PENDING_SPAWN_INTERVAL = 1.0f;            // 初期・追加スポーンの間隔（秒）
	const float INITIAL_SPAWN_INTERVAL = 5.0f;            // 通常スポーン間隔（秒）
}


namespace app
{
	namespace actor
	{
		EventCharacterSpawnManager* EventCharacterSpawnManager::instance_ = nullptr;

		EventCharacterSpawnManager::EventCharacterSpawnManager()
		{
		}


		EventCharacterSpawnManager::~EventCharacterSpawnManager()
		{
			CleanUp();
		}


		void EventCharacterSpawnManager::CleanUp()
		{
			for (auto& entry : activeEntries_)
			{
				if (entry.hpUI)
				{
					entry.hpUI->ClearTarget();
					hpUIPool_.Release(entry.hpUI);
				}
				if (auto* stone = dynamic_cast<StoneEventCharacter*>(entry.enemy))
				{
					stonePool_.Release(stone);
				}
				else if (auto* mushroom = dynamic_cast<MushroomEventCharacter*>(entry.enemy))
				{
					mushroomPool_.Release(mushroom);
				}
			}
			activeEntries_.clear();

			// シーン終了時にプールごと破棄
			stonePool_.Finalize();
			mushroomPool_.Finalize();
			hpUIPool_.Finalize();
		}

		void EventCharacterSpawnManager::SetPause(bool isPause)
		{
			isPause_ = isPause;

			// 現在生存中の全敵にポーズ状態を伝える
			for (auto& entry : activeEntries_)
			{
				if (auto* stone = dynamic_cast<StoneEventCharacter*>(entry.enemy))
				{
					stone->SetPause(isPause);
				}
				else if (auto* mushroom = dynamic_cast<MushroomEventCharacter*>(entry.enemy))
				{
					mushroom->SetPause(isPause);
				}
			}
		}


		bool EventCharacterSpawnManager::Start(app::actor::BattleCharacter* battleCharacter)
		{
			battleCharacter_ = battleCharacter;

			// スポーン地点管理を初期化（10地点をシャッフルしてキューに積む）
			quadrantManager_.Initialize();

			// 初期スポーンを予約
			pendingSpawnCount_ = INITIAL_SPAWN_COUNT;

			// 起動時に一括でオブジェクトを生成（以降はNewGOしない）
			stonePool_.Initialize();
			StoneEventCharacter::ResetInstanceCount();
			mushroomPool_.Initialize();
			MushroomEventCharacter::ResetInstanceCount();
			hpUIPool_.Initialize();

			return true;
		}


		void EventCharacterSpawnManager::Update()
		{
			if (isPause_) { return; }
			// EffectManagerの初期化完了を待つ
			if (!EffectManager::IsAvailable()) { return; }

			if (battleCharacter_ != nullptr)
			{
				attackPointManager_.Update(battleCharacter_->transform.position);
			}

			// 初期スポーン・敵死亡後の追加スポーンを1秒おきに1体ずつ処理
			if (pendingSpawnCount_ > 0)
			{
				const float deltaTime = g_gameTime->GetFrameDeltaTime();
				pendingSpawnTimer_ += deltaTime;

				if (pendingSpawnTimer_ >= PENDING_SPAWN_INTERVAL)
				{
					if (GetCurrentEnemyCount() < MAX_EVENT_CHARACTER)
					{
						pendingSpawnTimer_ = 0.0f;
						--pendingSpawnCount_;
						SpawnEventCharacter();
					}
				}
				return;
			}

			if (GetCurrentEnemyCount() >= MAX_EVENT_CHARACTER)
			{
				spawnTimer_ = 0.0f;
				return;
			}

			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			spawnTimer_ += deltaTime;

			if (spawnTimer_ < spawnInterval_) { return; }

			spawnTimer_ = 0.0f;
			SpawnEventCharacter();
		}


		float EventCharacterSpawnManager::GetSkeletonProbability() const
		{
			if (playerLevel_ < SKELETON_SPAWN_LEVEL) { return 0.0f; }

			const int stage = playerLevel_ - SKELETON_SPAWN_LEVEL;
			const float prob = SKELETON_BASE_PROBABILITY + SKELETON_PROBABILITY_INCREMENT * stage;
			return (std::min)(prob, MAX_SKELETON_PROBABILITY);
		}


		EnemyType EventCharacterSpawnManager::SelectEnemyType() const
		{
			const float skeletonProb = GetSkeletonProbability();
			const float roll = static_cast<float>(rand()) / RAND_MAX;

			if (roll < skeletonProb)
			{
				return EnemyType::SKELETON;
			}

			return (rand() % 2 == 0) ? EnemyType::STONE : EnemyType::MUSHROOM;
		}


		Vector3 EventCharacterSpawnManager::CalcSpawnPosition(const SpawnDirection& direction) const
		{
			// 原点（プレイヤー初期位置）を基準にした固定ワールド座標でスポーン
			// fieldEdge_ = フィールド端までの距離
			// halfEdge_  = その半分（辺中央・南北・東西用）
			const float e = fieldEdge_;
			const float h = fieldEdge_ * 0.5f;

			switch (direction)
			{
			// 4隅
			case SpawnDirection::NORTH_WEST:    return Vector3(-e, spawnPosY_, -e);
			case SpawnDirection::NORTH_EAST:    return Vector3(e, spawnPosY_, -e);
			case SpawnDirection::SOUTH_WEST:    return Vector3(-e, spawnPosY_, e);
			case SpawnDirection::SOUTH_EAST:    return Vector3(e, spawnPosY_, e);

			// 4辺の中央
			case SpawnDirection::NORTH:         return Vector3(0.0f, spawnPosY_, -e);
			case SpawnDirection::SOUTH:         return Vector3(0.0f, spawnPosY_, e);
			case SpawnDirection::WEST:          return Vector3(-e, spawnPosY_, 0.0f);
			case SpawnDirection::EAST:          return Vector3(e, spawnPosY_, 0.0f);

			// 北辺・南辺の1/4地点
			// 北辺を3分割した中間2点
			case SpawnDirection::NORTH_WEST_MID: return Vector3(-h, spawnPosY_, -e);
			case SpawnDirection::NORTH_EAST_MID: return Vector3(h, spawnPosY_, -e);

			default:
				return Vector3::Zero;
			}
		}


		int EventCharacterSpawnManager::GetCurrentEnemyCount() const
		{
			int count = 0;
			count += StoneEventCharacter::GetNum();
			count += MushroomEventCharacter::GetNum();
			// count += SkeletonEventCharacter::GetNum(); // スケルトンのクラスが出来たら追加
			return count;
		}


		void EventCharacterSpawnManager::SpawnEventCharacter()
		{
			// QuadrantManagerから次のスポーン地点を取得（使用済みフラグも内部で立てる）
			const SpawnDirection direction = quadrantManager_.GetNext();
			const Vector3 spawnPosition = CalcSpawnPosition(direction);

			/** スポーン時のY座標が反映されているかのtest */
			OutputDebugStringA(
				("spawnPosition Y: " + std::to_string(spawnPosition.y) + "\n").c_str()
			);

			const EnemyType type = SelectEnemyType();

			// デバッグ用：どの地点が選ばれたか出力
			const char* dirNames[] = {
				"NORTH_WEST", "NORTH_EAST", "SOUTH_WEST", "SOUTH_EAST",
				"NORTH", "SOUTH", "WEST", "EAST",
				"NORTH_WEST_MID", "NORTH_EAST_MID"
			};
			OutputDebugStringA(
				(std::string("Spawn direction: ") +
					dirNames[static_cast<int>(direction)] + "\n").c_str()
			);

			SpawnResult result;
			result.type = type;
			result.spawnPosition = spawnPosition;

			switch (type)
			{
			case EnemyType::STONE:
			{
				auto* stone = stonePool_.Acquire();
				if (!stone) { break; }

				stone->SetBattleCharacter(battleCharacter_);
				stone->SetAttckPointManager(&attackPointManager_);

				stone->transform.position = spawnPosition;
				stone->GetStateMachine()->transform.position = spawnPosition;

				stone->GetCharacterController()->SetPosition(spawnPosition);
				stone->GetCharacterController()->RequestTeleport();
				stone->SetPause(isPause_);

				auto* hpUI = hpUIPool_.Acquire();
				if (!hpUI) { break; }
				hpUI->SetTargetEnemy(stone);
				hpUI->SetPlayer(battleCharacter_);

				activeEntries_.push_back({ stone, hpUI });

				stone->AddOnDead([this, stone, hpUI, direction]()
					{
						hpUI->ClearTarget();
						hpUIPool_.Release(hpUI);
						stonePool_.Release(stone);
						quadrantManager_.Release(direction);
						++pendingSpawnCount_;
						activeEntries_.erase(
							std::remove_if(activeEntries_.begin(), activeEntries_.end(),
								[stone](const EnemyEntry& e) { return e.enemy == stone; }),
							activeEntries_.end());
					});
				result.stoneCharacter = stone;
				break;
			}
			case EnemyType::MUSHROOM:
			{
				auto* mushroom = mushroomPool_.Acquire();
				if (!mushroom) { break; }

				mushroom->SetBattleCharacter(battleCharacter_);
				mushroom->SetAttckPointManager(&attackPointManager_);

				mushroom->transform.position = spawnPosition;
				mushroom->GetStateMachine()->transform.position = spawnPosition;

				mushroom->GetCharacterController()->SetPosition(spawnPosition);
				mushroom->GetCharacterController()->RequestTeleport();

				mushroom->SetPause(isPause_);

				auto* hpUI = hpUIPool_.Acquire();
				if (!hpUI) { break; }
				hpUI->SetTargetEnemy(mushroom);
				hpUI->SetPlayer(battleCharacter_);

				activeEntries_.push_back({ mushroom, hpUI });

				mushroom->AddOnDead([this, mushroom, hpUI, direction]()
					{
						hpUI->ClearTarget();
						hpUIPool_.Release(hpUI);
						mushroomPool_.Release(mushroom);
						quadrantManager_.Release(direction);
						++pendingSpawnCount_;
						activeEntries_.erase(
							std::remove_if(activeEntries_.begin(), activeEntries_.end(),
								[mushroom](const EnemyEntry& e) { return e.enemy == mushroom; }),
							activeEntries_.end());
					});
				result.mushroomCharacter = mushroom;
				break;
			}
			case EnemyType::SKELETON:
				// SkeletonEventCharacterの生成処理
				// quadrantManager_.Release(direction) をAddOnDeadで呼ぶこと
				break;

			default:
				break;
			}

			if (result.IsValid())
			{
				if (onSpawned_) onSpawned_(result);

				// スポーン位置にポイントライトフラッシュを発動
				g_sceneLight->TriggerSpawnLight(spawnPosition);
			}

			//  // スポーンエフェクト再生
			//  if (effectManagerObject_ && result.IsValid())
			//  {
			//  	int spawnEffect = enEffectKind_None;
			//  
			//  	switch (type)
			//  	{
			//  	case EnemyType::STONE:
			//  		spawnEffect = enEffectKind_StoneSpawn;
			//  		break;
			//  	case EnemyType::MUSHROOM:
			//  		spawnEffect = enEffectKind_mushroomSpawn;
			//  		break;
			//  	default:
			//  		break;
			//  	}
			//  
			//  	if (spawnEffect != enEffectKind_None)
			//  	{
			//  		effectManagerObject_->PlayEffect(
			//  			spawnEffect,
			//  			spawnPosition,
			//  			Quaternion::Identity,
			//  			Vector3::One
			//  		);
			//  	}
			//  }
		}

		void EventCharacterSpawnManager::OnPlayerLevelUp(int newLevel)
		{
			playerLevel_ = newLevel;

			for (auto& entry : activeEntries_)
			{
				if (auto* stone = dynamic_cast<StoneEventCharacter*>(entry.enemy))
				{
					auto* s = stone->GetStatus()->As<StoneEventCharacterStatus>();
					if (s)
					{
						s->ApplyPhase(newLevel); // 戻り値に関わらず実行

						// phaseUIは常に現在のフェーズで更新する
						if (phaseUI_)
						{
							phaseUI_->SetPhaseCount(s->GetCurrentPhaseIndex());
						}
					}
				}


				else if (auto* mushroom = dynamic_cast<MushroomEventCharacter*>(entry.enemy))
				{
					auto* s = mushroom->GetStatus()->As<MushroomEventCharacterStatus>();
					if (s)
					{
						s->ApplyPhase(newLevel); // 戻り値に関わらず実行
						// phaseUIは常に現在のフェーズで更新する
						if (phaseUI_)
						{
							phaseUI_->SetPhaseCount(s->GetCurrentPhaseIndex());
						}
					}
				}
				
			}
		}

	}
}