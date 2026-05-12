#include "stdafx.h"
#include "EventCharacterSpawnManager.h"
#include "actor/EventCharacter.h"
#include "actor/BattleCharacter.h"
#include "ui/InGameUI.h"
#include <functional>


namespace
{
	const int MAX_EVENT_CHARACTER = 4;                    // 同時に存在できるイベントキャラクターの最大数
	const int INITIAL_SPAWN_COUNT = 4;                    // 初期スポーン数
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
		{}


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
					DeleteGO(entry.hpUI);
				}
				if (entry.enemy)
				{
					DeleteGO(entry.enemy);
				}
			}
			activeEntries_.clear();
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

			// 象限管理を初期化（4象限をシャッフルしてキューに積む）
			quadrantManager_.Initialize();

			// 初期スポーンを予約
			pendingSpawnCount_ = INITIAL_SPAWN_COUNT;

			return true;
		}


		void EventCharacterSpawnManager::Update()
		{
			if (isPause_) { return; }
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
			switch (direction)
			{
			case SpawnDirection::NORTH_WEST:
				return Vector3(-fieldEdge_, 0.0f, -fieldEdge_);

			case SpawnDirection::NORTH_EAST:
				return Vector3(fieldEdge_, 0.0f, -fieldEdge_);

			case SpawnDirection::SOUTH_WEST:
				return Vector3(-fieldEdge_, 0.0f, fieldEdge_);

			case SpawnDirection::SOUTH_EAST:
				return Vector3(fieldEdge_, 0.0f, fieldEdge_);

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
			// QuadrantManagerから次の象限を取得（使用済みフラグも内部で立てる）
			const SpawnDirection direction = quadrantManager_.GetNext();
			const Vector3 spawnPosition = CalcSpawnPosition(direction);
			const EnemyType type = SelectEnemyType();

			// デバッグ用：どの象限が選ばれたか出力
			const char* dirNames[] = { "NORTH_WEST", "NORTH_EAST", "SOUTH_WEST", "SOUTH_EAST" };
			OutputDebugStringA(
				(std::string("Spawn direction: ") +
					dirNames[static_cast<int>(direction)] + "\n").c_str()
			);

			SpawnResult result;
			result.type = type;

			switch (type)
			{
			case EnemyType::STONE:
			{
				auto* stone = NewGO<StoneEventCharacter>(
					static_cast<uint8_t>(ObjectPriority::Character), "StoneEventCharacter");
				stone->SetPause(isPause_);
				stone->transform.position = spawnPosition;
				stone->GetStateMachine()->transform.position = spawnPosition;

				auto* hpUI = NewGO<app::ui::EnemyHpUIObject>(static_cast<uint8_t>(ObjectPriority::EnemyUI));
				hpUI->SetTargetEnemy(stone);
				hpUI->SetPlayer(battleCharacter_);

				activeEntries_.push_back({ stone, hpUI });

				stone->AddOnDead([this, stone, hpUI, direction]()
					{
						hpUI->ClearTarget();
						DeleteGO(hpUI);
						DeleteGO(stone);
						quadrantManager_.Release(direction); // 象限を解放
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
				auto* mushroom = NewGO<MushroomEventCharacter>(
					static_cast<uint8_t>(ObjectPriority::Character), "MushroomEventCharacter");
				mushroom->SetPause(isPause_);
				mushroom->transform.position = spawnPosition;
				mushroom->GetStateMachine()->transform.position = spawnPosition;

				auto* hpUI = NewGO<app::ui::EnemyHpUIObject>(static_cast<uint8_t>(ObjectPriority::EnemyUI));
				hpUI->SetTargetEnemy(mushroom);
				hpUI->SetPlayer(battleCharacter_);

				activeEntries_.push_back({ mushroom, hpUI });

				mushroom->AddOnDead([this, mushroom, hpUI, direction]()
					{
						hpUI->ClearTarget();
						DeleteGO(hpUI);
						DeleteGO(mushroom);
						quadrantManager_.Release(direction); // 象限を解放
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

			if (onSpawned_ && result.IsValid())
			{
				onSpawned_(result);
			}
		}

	}
}