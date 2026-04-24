#pragma once

#include "Actor.h"
#include "actor/types.h"
#include "actor/QuadrantManager.h"
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


		/** スポーン結果 */
		struct SpawnResult
		{
			EnemyType type = EnemyType::STONE;

			app::actor::StoneEventCharacter* stoneCharacter = nullptr;
			app::actor::MushroomEventCharacter* mushroomCharacter = nullptr;
			/** TODO::スケルトンのクラスが出来たら追加 */

			bool IsValid() const
			{
				return (stoneCharacter != nullptr) || (mushroomCharacter != nullptr);
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

			/** イベントキャラクターをスポーンする */
			void SpawnEventCharacter();


		public:
			/** プレイヤーレベルを設定する */
			void SetPlayerLevel(int level) { playerLevel_ = level; }

			/** スポーン時のコールバックを設定する */
			void SetOnSpawned(SpawnCallback callback) { onSpawned_ = std::move(callback); }

			/** スポーン間隔を設定する */
			void SetSpawnInterval(float interval) { spawnInterval_ = interval; }

			void SetFieldEdge(float edge) { fieldEdge_ = edge; }

			///** フィールドの中心座標を設定する */
			//void SetFieldCenter(const Vector3& center) { fieldCenter_ = center; }
			//
			///** フィールドサイズを設定する */
			//void SetFieldSize(float size) { fieldSize_ = size; }
			//
			///** スポーンのばらつき幅を設定する */
			//void SetSpawnScatter(float scatter) { spawnScatter_ = scatter; }


		private:
			/** スケルトンの出現確率を計算する (Lv1～Lv5 : 0% , Lv6以降:段階的に上昇) */
			float GetSkeletonProbability() const;

			/** スポーンするイベントキャラクターの種類を決定する */
			EnemyType SelectEnemyType() const;

			/** スポーン座標を計算する */
			Vector3 CalcSpawnPosition(const SpawnDirection& direction) const;

			/** 現在スポーンしているイベントキャラクターの数を取得する */
			int GetCurrentEnemyCount() const;


		private:
			QuadrantManager quadrantManager_;                          // 象限管理
			//Vector3 fieldCenter_ = Vector3(0, 0, 0);                  // フィールドの中心座標
			//float fieldSize_ = 20000.0f;                              // フィールドの一辺のサイズ
			//float spawnScatter_ = 1000.0f;                            // 象限中心からのランダムばらつき幅

			float fieldEdge_ = 20000.0f; // 原点からスポーン地点までの距離（フィールド端寄り）

			int playerLevel_ = 1;
			float spawnInterval_ = 5.0f;                              // スポーン間隔（秒）
			float spawnTimer_ = 0.0f;                                 // スポーンタイマー
			int pendingSpawnCount_ = 0;                               // 次フレームでスポーンする残数
			float pendingSpawnTimer_ = 1.0f;                          // 初期・追加スポーンのインターバルタイマー（初回は即スポーン）

			app::actor::BattleCharacter* battleCharacter_ = nullptr;  // プレイヤーキャラクターへの参照
			SpawnCallback onSpawned_ = nullptr;                       // スポーン時のコールバック
		};

	}
}