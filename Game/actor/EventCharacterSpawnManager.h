#pragma once

#include "Actor.h"
#include "actor/types.h"
#include <functional>
#include <utility>


namespace app
{

	namespace actor
	{
        class BattleCharacter;
		class StoneEventCharacter;
		class MushroomEventCharacter;

		/** イベントキャラクターの種類 */
		enum class EnemyType
		{
			STONE,
			MUSHROOM,
			SKELETON,
			MAX
		};


		/** イベントキャラクターのスポーンする方向 */
		enum class SpawnDirection
		{
			NORTH,
			SOUTH,
			EAST,
			WEST,
			MAX
		};


		/** スポーン結果 */
		struct SpawnResult
		{
			EnemyType type = EnemyType::STONE;


			app::actor::StoneEventCharacter* stoneCharacter = nullptr;
			app::actor::MushroomEventCharacter* mushroomCharacter = nullptr;
			/** TODO::スケルトンのクラスが出来たら追加 */


			bool IsValid() const
			{
				return (stoneCharacter != nullptr) || (mushroomCharacter != nullptr); // || (skeletonCharacter != nullptr);
			}


		};


		/** イベントキャラクターのスポーンを管理するクラス */
		class EventCharacterSpawnManager
		{
		public:
			using SpawnCallback = std::function<void(const SpawnResult&)>;


		public:
			EventCharacterSpawnManager();
			~EventCharacterSpawnManager();


			bool Start(app::actor::BattleCharacter* battleCharacter);
			void Update();


		private:
			int playerLevel_ = 1;
			float spawnInterval_ = 5.0f; // スポーン間隔（秒）
			float spawnTimer_ = 0.0f; // スポーンタイマー

			app::actor::BattleCharacter* battleCharacter_ = nullptr; // プレイヤーキャラクターへの参照
			SpawnCallback onSpawned_ = nullptr; // スポーン時のコールバック


		public:
			/** プレイヤーレベルを設定する */
			void SetPlayerLevel(int level) { playerLevel_ = level; }


			/** スポーン時のコールバックを設定する */
           void SetOnSpawned(SpawnCallback callback) { onSpawned_ = std::move(callback); }


			/** スポーン間隔を設定する */
			void SetSpawnInterval(float interval) { spawnInterval_ = interval; }


		private:
			/** スケルトンの出現確率を計算する (Lv1～Lv5 : 0% , Lv6以降:段階的に上昇) */
			float GetSkeletonProbability() const;


			/** スポーンするイベントキャラクターの種類を決定する */
			EnemyType SelectEnemyType() const;


			/** スポーンする方向をランダムに決定する */
			SpawnDirection GetRandomSpawnDirection() const;


			/** スポーン座標を計算する */
			Vector3 CalcSpawnPosition(const SpawnDirection& direction) const;


			/** 現在スポーンしているイベントキャラクターの数を取得する */
			int GetCurrentEnemyCount() const;


			/** イベントキャラクターをスポーンする */
			void SpawnEventCharacter();


		};
	}
}