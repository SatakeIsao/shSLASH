#pragma once

#include "Actor.h"
#include "actor/types.h"
#include "actor/EnemyPool.h"
#include "actor/QuadrantManager.h"
#include "actor/EnemyAttackPointManager.h"
#include <functional>
#include <utility>

namespace app
{
	namespace ui { class EnemyHpUIObject; class PhaseUI; }
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
			app::actor::StoneEventCharacter*    stoneCharacter    = nullptr;
			app::actor::MushroomEventCharacter* mushroomCharacter = nullptr;
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
			void SetPlayerLevel(int level)             { playerLevel_ = level; }

			/** スポーン時のコールバックを設定する */
			void SetOnSpawned(SpawnCallback callback)  { onSpawned_ = std::move(callback); }

			/** スポーン間隔を設定する */
			void SetSpawnInterval(float interval)      { spawnInterval_ = interval; }

			/** フィールド端までの距離を設定する */
			void SetFieldEdge(float edge)              { fieldEdge_ = edge; }

			/** スポーン時のY座標を設定する */
			void SetSpawnPosY(float y)                 { spawnPosY_ = y; }

			/** フェーズUIを設定する */
			void SetPhaseUI(app::ui::PhaseUI* phaseUI) { phaseUI_ = phaseUI; }

			/** 生存中の全敵とHPバーをDeleteGOする（シーン破棄時に呼ぶ） */
			void CleanUp();

			/** ポーズ状態を設定する */
			void SetPause(bool isPause);

			/** チュートリアルモードON/OFF（ONにするとUpdate内の自動スポーンを全停止） */
			void SetTutorialMode(bool v)               { isTutorialMode_ = v; }

			/** オープニングシーケンス完了フラグを設定する */
			void SetOpeningSequenceDone(bool v)        { isOpeningSequenceDone_ = v; }

			/** チュートリアル用：自動スポーンを停止する */
			void ResetPendingSpawn()                   { pendingSpawnCount_ = 0; pendingSpawnTimer_ = 0.0f; }

			/** チュートリアル用：指定の種類・座標・向きに1体スポーンする（死亡時に再スポーンしない） */
			void SpawnFixed(EnemyType type, Vector3 position, Quaternion rotation = Quaternion::Identity);

			/** チュートリアル中に敵を凍結/解除する（全アクティブ敵に即時反映） */
			void SetTutorialEnemyFrozen(bool frozen);

			/** 全アクティブ敵HPバーのブラー前描画フラグを一括設定する */
			void SetAllHpBarsPreBlurRender(bool v);

			/** 現在アクティブな敵の数を返す */
			int GetActiveEnemyCount() const            { return static_cast<int>(activeEntries_.size()); }

			/** origin から forward 方向・視野角 halfAngleDeg 度以内で最も正面に近い敵の位置を返す */
			bool FindNearestEnemyInCone(const Vector3& origin, const Vector3& forward, float halfAngleDeg, Vector3& outPos) const;

			/** プレイヤーレベルが上がったときに呼ばれる */
			void OnPlayerLevelUp(int newLevel);

			/** 攻撃ポイントマネージャーを取得する */
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
				IGameObject*              enemy = nullptr;
				app::ui::EnemyHpUIObject* hpUI  = nullptr;
			};

			/** 外部参照 */
			app::actor::BattleCharacter* battleCharacter_ = nullptr;
			app::ui::PhaseUI*            phaseUI_         = nullptr;
			SpawnCallback                onSpawned_       = nullptr;

			/** スポーン設定 */
			float fieldEdge_     = 20000.0f;
			float spawnInterval_ = 5.0f;
			float spawnPosY_     = 0.0f;
			int   maxEnemyCount_ = 10;

			/** ゲーム進行状態 */
			int playerLevel_       = 1;
			int currentPhaseIndex_ = 1;

			/** スポーンタイマー */
			float spawnTimer_        = 0.0f;
			int   pendingSpawnCount_ = 0;
			float pendingSpawnTimer_ = 1.0f;

			/** フラグ */
			bool isPause_               = false;
			bool isTutorialMode_        = false;
			bool tutorialEnemyFrozen_   = false;
			/** バトルシーケンス（カウントダウン）完了前はスポーンを抑制する */
			bool isOpeningSequenceDone_ = false;

			/** マネージャー・プール */
			static constexpr int MAX_EVENT_CHARACTER = 20;

			std::vector<EnemyEntry>                                        activeEntries_;
			QuadrantManager                                                quadrantManager_;
			EnemyAttackPointManager                                        attackPointManager_;
			EnemyPool<StoneEventCharacter,            MAX_EVENT_CHARACTER> stonePool_;
			EnemyPool<MushroomEventCharacter,         MAX_EVENT_CHARACTER> mushroomPool_;
			EnemyPool<app::ui::EnemyHpUIObject,       MAX_EVENT_CHARACTER> hpUIPool_;

		/** シングルトン */
		public:
			static void Initialize()
			{
				if (instance_ == nullptr)
				{
					instance_ = new EventCharacterSpawnManager();
				}
			}

			static EventCharacterSpawnManager& Get()
			{
				return *instance_;
			}

			static bool IsAvailable()
			{
				return instance_ != nullptr;
			}

			static void Finalize()
			{
				if (instance_ != nullptr)
				{
					delete instance_;
					instance_ = nullptr;
				}
			}

		private:
			static EventCharacterSpawnManager* instance_;
		};
	}
}
