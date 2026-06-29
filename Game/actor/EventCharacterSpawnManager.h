#pragma once

#include "Actor.h"
#include "actor/types.h"
#include "actor/EnemyPool.h"
#include "actor/QuadrantManager.h"
#include "EnemyAttackPointManager.h"
#include <functional>
#include <utility>

namespace
{
	constexpr int MAX_EVENT_CHARACTER = 20;
}

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

			Vector3 spawnPosition = Vector3::Zero;

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

			/** ポーズ状態を設定する */
			void SetPause(bool isPause);

			/** スポーン時のY座標を設定 */
			void SetSpawnPosY(float y) { spawnPosY_ = y; }

			void SetPhaseUI(PhaseUI* phaseUI) { phaseUI_ = phaseUI; }

			/** チュートリアル用：自動スポーンを停止する */
			void ResetPendingSpawn() { pendingSpawnCount_ = 0; pendingSpawnTimer_ = 0.0f; }

			/** チュートリアル用：指定の種類・座標・向きに1体スポーンする（死亡時に再スポーンしない） */
			void SpawnFixed(EnemyType type, Vector3 position, Quaternion rotation = Quaternion::Identity);

			/** チュートリアルモードON/OFF（ONにするとUpdate内の自動スポーンを全停止） */
			void SetTutorialMode(bool v) { isTutorialMode_ = v; }
			void SetOpeningSequenceDone(bool v) { isOpeningSequenceDone_ = v; }

			/** チュートリアル中に敵を凍結/解除する（全アクティブ敵に即時反映） */
			void SetTutorialEnemyFrozen(bool frozen);

			/** 全アクティブ敵HPバーのブラー前描画フラグを一括設定する */
			void SetAllHpBarsPreBlurRender(bool v);

			/** 現在アクティブな敵の数を返す */
			int GetActiveEnemyCount() const { return static_cast<int>(activeEntries_.size()); }

			/** プレイヤーレベルが上がったときに呼ばれる */
			void OnPlayerLevelUp(int newLevel);

			EnemyAttackPointManager& GetAttackPointManager() { return attackPointManager_; }

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

			QuadrantManager quadrantManager_;                         // スポーン地点管理

			PhaseUI* phaseUI_ = nullptr;							  // フェーズUIへの参照（スポーン間隔の表示に使う）

			float fieldEdge_ = 20000.0f;							  // 原点からスポーン地点までの距離（フィールド端寄り）

			int playerLevel_ = 1;
			int maxEnemyCount_ = 10;                                  // 現在フェーズの最大敵数
			int currentPhaseIndex_ = 1;                               // 現在のフェーズ番号（1始まり）
			float spawnInterval_ = 5.0f;                              // スポーン間隔（秒）
			float spawnTimer_ = 0.0f;                                 // スポーンタイマー
			int pendingSpawnCount_ = 0;                               // 次フレームでスポーンする残数
			float pendingSpawnTimer_ = 1.0f;                          // 初期・追加スポーンのインターバルタイマー（初回は即スポーン）
			float spawnPosY_ = 0.0f;									  // スポーンY座標
			bool isPause_ = false;
			bool isTutorialMode_ = false;
			bool tutorialEnemyFrozen_ = false;
			/** バトルシーケンス（カウントダウン）完了前はスポーンを抑制する */
			bool isOpeningSequenceDone_ = false;

			app::actor::BattleCharacter* battleCharacter_ = nullptr;  // プレイヤーキャラクターへの参照
			SpawnCallback onSpawned_ = nullptr;                       // スポーン時のコールバック

			EnemyAttackPointManager attackPointManager_;			  // 敵の攻撃ポイント管理

			EnemyPool<StoneEventCharacter, MAX_EVENT_CHARACTER> stonePool_;
			EnemyPool<MushroomEventCharacter, MAX_EVENT_CHARACTER> mushroomPool_;
			EnemyPool<app::ui::EnemyHpUIObject, MAX_EVENT_CHARACTER> hpUIPool_;

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