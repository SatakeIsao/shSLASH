#include "stdafx.h"
#include "EventCharacterSpawnManager.h"
#include "actor/EventCharacter.h"
#include "actor/BattleCharacter.h"
#include "ui/InGameUI.h"
#include <functional>


namespace
{
	const int MAX_EVENT_CHARACTER = 4; // 同時に存在できるイベントキャラクターの最大数
	const int INITIAL_SPAWN_COUNT = 2; // 初期スポーン数
	const int MAX_PLAYER_LEVEL = 10; // プレイヤーレベルの最大値
	const int SKELETON_SPAWN_LEVEL = 6; // スケルトンがスポーンし始めるプレイヤーレベル
	const float SKELETON_BASE_PROBABILITY = 0.1f; // スケルトンの基本出現確率 (Lv6で10%)
	const float SKELETON_PROBABILITY_INCREMENT = 0.1f; // プレイヤーレベルが1上がるごとにスケルトンの出現確率が上昇する量
	const float MAX_SKELETON_PROBABILITY = 0.6f; // スケルトンの最大出現確率 (60%)
	const float SPAWN_OFFSET_DISTANCE = 5000.0f; // プレイヤーからイベントキャラクターがスポーンする距離
	const float INITIAL_SPAWN_INTERVAL = 5.0f; // 初期スポーン間隔（秒）

}


namespace app
{

	namespace actor
	{

		EventCharacterSpawnManager::EventCharacterSpawnManager()
		{
		}


		EventCharacterSpawnManager::~EventCharacterSpawnManager()
		{
		}


		bool EventCharacterSpawnManager::Start(app::actor::BattleCharacter* battleCharacter)
		{
			battleCharacter_ = battleCharacter;


			return true;
		}


		void EventCharacterSpawnManager::Update()
		{
			if (GetCurrentEnemyCount() >= MAX_EVENT_CHARACTER)
			{
				spawnTimer_ = 0.0f; // タイマーをリセットして次のスポーンを待つ

				return;
			}

			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			spawnTimer_ += deltaTime;

			if (spawnTimer_ < spawnInterval_) { return; }

			spawnTimer_ = 0.0f; // タイマーをリセット
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
			const float roll = static_cast<float>(rand()) / RAND_MAX; // 0.0f～1.0fの乱数


			if (roll < skeletonProb)
			{
				return EnemyType::SKELETON;
			}


			return (rand() % 2 == 0) ? EnemyType::STONE : EnemyType::MUSHROOM;
		}


		SpawnDirection EventCharacterSpawnManager::GetRandomSpawnDirection() const
		{
			return static_cast<SpawnDirection>(rand() % 4);
		}


		Vector3 EventCharacterSpawnManager::CalcSpawnPosition(const SpawnDirection& direction) const
		{
			const Vector3 playerPosition = Vector3::Zero; // プレイヤーの現在位置 (仮)


			switch (direction)
			{
			case SpawnDirection::NORTH:
				return playerPosition + Vector3(0, 0, -SPAWN_OFFSET_DISTANCE);

			case SpawnDirection::SOUTH:
				return playerPosition + Vector3(0, 0, SPAWN_OFFSET_DISTANCE);

			case SpawnDirection::EAST:
				return playerPosition + Vector3(SPAWN_OFFSET_DISTANCE, 0, 0);

			case SpawnDirection::WEST:
				return playerPosition + Vector3(-SPAWN_OFFSET_DISTANCE, 0, 0);

			default:
				return playerPosition;
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
			const SpawnDirection direction = GetRandomSpawnDirection();
			const Vector3 spawnPosition = CalcSpawnPosition(direction);
			const EnemyType type = SelectEnemyType();

			SpawnResult result;
			result.type = type;

			switch (type)
			{
			case EnemyType::STONE:
			{
				auto* stone = NewGO<StoneEventCharacter>(
					static_cast<uint8_t>(ObjectPriority::Character), "StoneEventCharacter");
				stone->transform.position = spawnPosition;

				auto* hpUI = NewGO<app::ui::EnemyHpUIObject>(static_cast<uint8_t>(ObjectPriority::EnemyUI));
				hpUI->SetTargetEnemy(stone);
				hpUI->SetPlayer(battleCharacter_);

				stone->SetOnDead([this, stone, hpUI]()
					{
						DeleteGO(hpUI);
						DeleteGO(stone);
						SpawnEventCharacter();
					});
				result.stoneCharacter = stone;
				break;
			}
			case EnemyType::MUSHROOM:
			{
				auto* mushroom = NewGO<MushroomEventCharacter>(
					static_cast<uint8_t>(ObjectPriority::Character), "MushroomEventCharacter");
				mushroom->transform.position = spawnPosition;

				auto* hpUI = NewGO<app::ui::EnemyHpUIObject>(static_cast<uint8_t>(ObjectPriority::EnemyUI));
				hpUI->SetTargetEnemy(mushroom);
				hpUI->SetPlayer(battleCharacter_);

				mushroom->SetOnDead([this, mushroom, hpUI]()
					{
						DeleteGO(hpUI);
						DeleteGO(mushroom);
						SpawnEventCharacter();
					});
				result.mushroomCharacter = mushroom;
				break;
			}
			case EnemyType::SKELETON:
				// SkeletonEventCharacterの生成処理
				break;

			default:
				break;
			}


			if (onSpawned_ && result.IsValid()){
				onSpawned_(result);
			}
		}


	}
}