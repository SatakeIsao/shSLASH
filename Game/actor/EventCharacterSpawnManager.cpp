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
	const int INITIAL_SPAWN_COUNT = 10;                   // 初期スポーン数（フェーズ1）
	const int PHASE_MAX_ENEMIES[] = { 10, 15, 20 };       // フェーズ1/2/3の最大敵数
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

			const bool actualPause = isPause_ || tutorialEnemyFrozen_;

			for (auto& entry : activeEntries_)
			{
				if (auto* stone = dynamic_cast<StoneEventCharacter*>(entry.enemy))
				{
					stone->SetPause(actualPause);
				}
				else if (auto* mushroom = dynamic_cast<MushroomEventCharacter*>(entry.enemy))
				{
					mushroom->SetPause(actualPause);
				}
			}
		}


		void EventCharacterSpawnManager::SetTutorialEnemyFrozen(bool frozen)
		{
			tutorialEnemyFrozen_ = frozen;

			const bool actualPause = isPause_ || tutorialEnemyFrozen_;

			for (auto& entry : activeEntries_)
			{
				if (auto* stone = dynamic_cast<StoneEventCharacter*>(entry.enemy))
				{
					stone->SetPause(actualPause);
				}
				else if (auto* mushroom = dynamic_cast<MushroomEventCharacter*>(entry.enemy))
				{
					mushroom->SetPause(actualPause);
				}
			}
		}

		void EventCharacterSpawnManager::SetAllHpBarsPreBlurRender(bool v)
		{
			for (auto& entry : activeEntries_)
			{
				if (entry.hpUI)
					entry.hpUI->SetPreBlurRender(v);
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

			// チュートリアルモードでもアタック/待機ポイントの座標更新とクールダウン計算は必要
			if (battleCharacter_ != nullptr)
			{
				attackPointManager_.Update(battleCharacter_->transform.position);
			}

			if (isTutorialMode_) { return; }
			// バトルシーケンス（カウントダウン）終了前はスポーンしない
			if (!isOpeningSequenceDone_) { return; }
			// EffectManagerの初期化完了を待つ
			if (!EffectManager::IsAvailable()) { return; }

			// 初期スポーン・敵死亡後の追加スポーンを1秒おきに1体ずつ処理
			if (pendingSpawnCount_ > 0)
			{
				const float deltaTime = g_gameTime->GetFrameDeltaTime();
				pendingSpawnTimer_ += deltaTime;

				if (pendingSpawnTimer_ >= PENDING_SPAWN_INTERVAL)
				{
					if (GetCurrentEnemyCount() < maxEnemyCount_)
					{
						pendingSpawnTimer_ = 0.0f;
						--pendingSpawnCount_;
						SpawnEventCharacter();
					}
				}
				return;
			}

			if (GetCurrentEnemyCount() >= maxEnemyCount_)
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

			// 北辺の1/4・3/4地点
			case SpawnDirection::NORTH_WEST_MID: return Vector3(-h, spawnPosY_, -e);
			case SpawnDirection::NORTH_EAST_MID: return Vector3(h, spawnPosY_, -e);

			// 南辺の1/4・3/4地点
			case SpawnDirection::SOUTH_WEST_MID: return Vector3(-h, spawnPosY_, e);
			case SpawnDirection::SOUTH_EAST_MID: return Vector3(h, spawnPosY_, e);

			// 西辺の1/4・3/4地点
			case SpawnDirection::WEST_NORTH_MID: return Vector3(-e, spawnPosY_, -h);
			case SpawnDirection::WEST_SOUTH_MID: return Vector3(-e, spawnPosY_, h);

			// 東辺の1/4・3/4地点
			case SpawnDirection::EAST_NORTH_MID: return Vector3(e, spawnPosY_, -h);
			case SpawnDirection::EAST_SOUTH_MID: return Vector3(e, spawnPosY_, h);

			// 北辺の外寄り中間点（fieldEdge_ の 3/4 地点）
			case SpawnDirection::NORTH_FAR_WEST: return Vector3(-e * 0.75f, spawnPosY_, -e);
			case SpawnDirection::NORTH_FAR_EAST: return Vector3(e * 0.75f, spawnPosY_, -e);

			// 南辺の外寄り中間点
			case SpawnDirection::SOUTH_FAR_WEST: return Vector3(-e * 0.75f, spawnPosY_, e);
			case SpawnDirection::SOUTH_FAR_EAST: return Vector3(e * 0.75f, spawnPosY_, e);

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
				"NORTH_WEST_MID", "NORTH_EAST_MID",
				"SOUTH_WEST_MID", "SOUTH_EAST_MID",
				"WEST_NORTH_MID", "WEST_SOUTH_MID",
				"EAST_NORTH_MID", "EAST_SOUTH_MID",
				"NORTH_FAR_WEST", "NORTH_FAR_EAST",
				"SOUTH_FAR_WEST", "SOUTH_FAR_EAST"
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
				if (!hpUI) { stonePool_.Release(stone); break; }
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
				if (!hpUI) { mushroomPool_.Release(mushroom); break; }
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

		void EventCharacterSpawnManager::SpawnFixed(EnemyType type, Vector3 position, Quaternion rotation)
		{
			SpawnResult result;
			result.type = type;
			result.spawnPosition = position;

			const bool spawnPaused = isPause_ || tutorialEnemyFrozen_;

			switch (type)
			{
			case EnemyType::STONE:
			{
				auto* stone = stonePool_.Acquire();
				if (!stone) { break; }

				stone->SetBattleCharacter(battleCharacter_);
				stone->SetAttckPointManager(&attackPointManager_);
				stone->transform.localPosition = position;
				stone->transform.localRotation = rotation;
				stone->transform.localScale = Vector3::One;
				stone->transform.position = position;
				stone->transform.rotation = rotation;
				stone->transform.scale = Vector3::One;
				stone->transform.UpdateTransform();
				stone->GetStateMachine()->transform.position = position;
				stone->GetStateMachine()->transform.rotation = rotation;
				stone->GetStateMachine()->transform.scale = Vector3::One;
				stone->GetCharacterController()->SetPosition(position);
				stone->GetCharacterController()->RequestTeleport();
				stone->SetPause(spawnPaused);

				auto* hpUI = hpUIPool_.Acquire();
				if (!hpUI) { break; }
				hpUI->SetTargetEnemy(stone);
				hpUI->SetPlayer(battleCharacter_);

				activeEntries_.push_back({ stone, hpUI });

				stone->AddOnDead([this, stone, hpUI]()
					{
						hpUI->ClearTarget();
						hpUIPool_.Release(hpUI);
						stonePool_.Release(stone);
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
				mushroom->transform.localPosition = position;
				mushroom->transform.localRotation = rotation;
				mushroom->transform.localScale = Vector3::One;
				mushroom->transform.position = position;
				mushroom->transform.rotation = rotation;
				mushroom->transform.scale = Vector3::One;
				mushroom->transform.UpdateTransform();
				mushroom->GetStateMachine()->transform.position = position;
				mushroom->GetStateMachine()->transform.rotation = rotation;
				mushroom->GetStateMachine()->transform.scale = Vector3::One;
				mushroom->GetCharacterController()->SetPosition(position);
				mushroom->GetCharacterController()->RequestTeleport();
				mushroom->SetPause(spawnPaused);

				auto* hpUI = hpUIPool_.Acquire();
				if (!hpUI) { break; }
				hpUI->SetTargetEnemy(mushroom);
				hpUI->SetPlayer(battleCharacter_);

				activeEntries_.push_back({ mushroom, hpUI });

				mushroom->AddOnDead([this, mushroom, hpUI]()
					{
						hpUI->ClearTarget();
						hpUIPool_.Release(hpUI);
						mushroomPool_.Release(mushroom);
						activeEntries_.erase(
							std::remove_if(activeEntries_.begin(), activeEntries_.end(),
								[mushroom](const EnemyEntry& e) { return e.enemy == mushroom; }),
							activeEntries_.end());
					});
				result.mushroomCharacter = mushroom;
				break;
			}
			default:
				break;
			}

			if (result.IsValid())
			{
				// onSpawned_ 内で SetGravity() が呼ばれるため、床検出より先に実行する
				if (onSpawned_) onSpawned_(result);

				// onSpawned_ で SetGravity() が確定した後、スポーンY+500 の高位置から
				// 重力スイープして床面Y座標を正確に求める
				auto groundSettle = [](auto* character, const Vector3& spawnPos) -> Vector3
				{
					auto* cc = character->GetCharacterController();
					const Vector3 abovePos(spawnPos.x, spawnPos.y + 500.0f, spawnPos.z);
					cc->SetPosition(abovePos);
					cc->RequestTeleport();
					cc->Execute(abovePos, 1.0f / 60.0f);              // テレポート処理
					cc->ResetGroundState();
					cc->Execute(cc->GetPosition(), 1.0f / 60.0f * 200.0f); // 重力スイープで接地
					return cc->GetPosition();
				};

				if (result.stoneCharacter)
				{
					const Vector3 gp = groundSettle(result.stoneCharacter, result.spawnPosition);
					result.stoneCharacter->transform.localPosition = gp;
					result.stoneCharacter->transform.position = gp;
					result.stoneCharacter->transform.UpdateTransform();
					result.stoneCharacter->GetStateMachine()->transform.position = gp;
					if (result.stoneCharacter->GetModelRender())
					{
						result.stoneCharacter->GetModelRender()->SetTRS(
							gp,
							result.stoneCharacter->transform.rotation,
							result.stoneCharacter->transform.scale);
						result.stoneCharacter->GetModelRender()->Update();
					}
				}
				if (result.mushroomCharacter)
				{
					const Vector3 gp = groundSettle(result.mushroomCharacter, result.spawnPosition);
					result.mushroomCharacter->transform.localPosition = gp;
					result.mushroomCharacter->transform.position = gp;
					result.mushroomCharacter->transform.UpdateTransform();
					result.mushroomCharacter->GetStateMachine()->transform.position = gp;
					if (result.mushroomCharacter->GetModelRender())
					{
						result.mushroomCharacter->GetModelRender()->SetTRS(
							gp,
							result.mushroomCharacter->transform.rotation,
							result.mushroomCharacter->transform.scale);
						result.mushroomCharacter->GetModelRender()->Update();
					}
				}
				g_sceneLight->TriggerSpawnLight(position);
			}
		}


		void EventCharacterSpawnManager::OnPlayerLevelUp(int newLevel)
		{
			playerLevel_ = newLevel;

			int newPhaseIndex = currentPhaseIndex_;

			for (auto& entry : activeEntries_)
			{
				if (auto* stone = dynamic_cast<StoneEventCharacter*>(entry.enemy))
				{
					auto* s = stone->GetStatus()->As<StoneEventCharacterStatus>();
					if (s)
					{
						s->ApplyPhase(newLevel);
						newPhaseIndex = s->GetCurrentPhaseIndex();
						if (phaseUI_)
						{
							phaseUI_->SetPhaseCount(newPhaseIndex);
						}
					}
				}
				else if (auto* mushroom = dynamic_cast<MushroomEventCharacter*>(entry.enemy))
				{
					auto* s = mushroom->GetStatus()->As<MushroomEventCharacterStatus>();
					if (s)
					{
						s->ApplyPhase(newLevel);
						newPhaseIndex = s->GetCurrentPhaseIndex();
						if (phaseUI_)
						{
							phaseUI_->SetPhaseCount(newPhaseIndex);
						}
					}
				}
			}

			// フェーズが上がった場合、最大敵数を増やして追加スポーンを予約する
			if (newPhaseIndex > currentPhaseIndex_)
			{
				currentPhaseIndex_ = newPhaseIndex;
				const int phaseIdx = (std::min)(newPhaseIndex - 1, 2); // 配列範囲クランプ
				const int newMax = PHASE_MAX_ENEMIES[phaseIdx];
				const int delta = newMax - maxEnemyCount_;
				if (delta > 0)
				{
					maxEnemyCount_ = newMax;
					pendingSpawnCount_ += delta;
				}
			}
		}

	}
}