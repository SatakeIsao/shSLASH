#pragma once

#include "Actor.h"
#include "actor/types.h"
#include "actor/QuadrantManager.h"
#include <functional>
#include <utility>


namespace app
{
	namespace ui { class EnemyHpUIObject; }
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

			/** 生存中の全敵とHPバーをDeleteGOする（シーン破棄時に呼ぶ） */
			void CleanUp();

			void SetPause(bool isPause);


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
			struct EnemyEntry
			{
				IGameObject* enemy = nullptr;
				app::ui::EnemyHpUIObject* hpUI = nullptr;
			};
			std::vector<EnemyEntry> activeEntries_;

			QuadrantManager quadrantManager_;                          // 象限管理

			float fieldEdge_ = 20000.0f; // 原点からスポーン地点までの距離（フィールド端寄り）

			int playerLevel_ = 1;
			float spawnInterval_ = 5.0f;                              // スポーン間隔（秒）
			float spawnTimer_ = 0.0f;                                 // スポーンタイマー
			int pendingSpawnCount_ = 0;                               // 次フレームでスポーンする残数
			float pendingSpawnTimer_ = 1.0f;                          // 初期・追加スポーンのインターバルタイマー（初回は即スポーン）
			bool isPause_ = false;

			app::actor::BattleCharacter* battleCharacter_ = nullptr;  // プレイヤーキャラクターへの参照
			SpawnCallback onSpawned_ = nullptr;                       // スポーン時のコールバック

			/**
			* シングルトン用
			*/
		public:
			/**
			 * インスタンスを作る
			 */
			static void Initialize()
			{
				if (instance_ == nullptr)
				{
					instance_ = new EventCharacterSpawnManager();
				}
			}


			/**
			 * インスタンスを取得
			 */
			static EventCharacterSpawnManager& Get()
			{
				return *instance_;
			}


			/**
			 * インスタンスが有効か
			 */
			static bool IsAvailable()
			{
				return instance_ != nullptr;
			}


			/**
			 * インスタンスを破棄
			 */
			static void Finalize()
			{
				if (instance_ != nullptr)
				{
					delete instance_;
					instance_ = nullptr;
				}
			}

		private:
			/** シングルトンインスタンス */
			static EventCharacterSpawnManager* instance_;
		};
	}
}