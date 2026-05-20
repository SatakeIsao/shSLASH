#pragma once
#include "Layout.h"
#include "actor/BattleCharacter.h"
#include "actor/EventCharacter.h"
#include "actor/EnemyPool.h"

namespace app
{
	namespace ui
	{
		class LevelUpUIObject;

		class TimerUIObject : public IGameObject
		{
		private:
			std::unique_ptr<app::ui::Layout> layout_;
			float timer_ = 0.0f;

			bool isCounting_ = true;


		public:
			TimerUIObject();
			~TimerUIObject();
		public:
			void Update();
			void Render(RenderContext& rc);
			// タイマー操作用の関数
			float GetTimer() const { return timer_; }
			void SetTimer(float time) { timer_ = time; }

			void StartTimer() { isCounting_ = true; }
			void StopTimer() { isCounting_ = false; }
			bool IsTimeUp() const { return timer_ <= 0.0f; }

		};




		/*************************************************/


		class HpUIObject : public IGameObject
		{
		protected:
			float damageDelayTimer_ = 0.0f;
			//ディレイHPバーがHPバーに線形補間で追従する時用
			float lerpVal_ = 0.5f;
			float damagePosX_ = 0.0f;
			float damageScaleX_ = 0.0f;
			int index_ = 0;
			int previousLevel_ = 0;
			int levelUpIndex_ = 0;
			int level_ = 0;

			bool isCounting_ = true;
			bool isLevelUpPending_ = false; // レベルアップ演出待ちフラグ


		public:
			HpUIObject() {};
			virtual ~HpUIObject() = default;
		public:
			virtual void Update() = 0;
			virtual void Render(RenderContext& rc) = 0;
		};




		/*************************************************/


		class PlayerHpUIObject : public HpUIObject
		{
		private:
			CircularGaugeRender hpGauge_;
			CircularGaugeRender bgCircle_;
			SpriteRender icon_;
			std::unique_ptr<app::ui::Layout> layout_;
			LevelUpUIObject* levelUpUIObject_ = nullptr;
			app::actor::BattleCharacter* player_ = nullptr;
			
			/** 無敵時間 */
			float invincibleTimer_ = 0.0f;
			float blinkTimer_ = 0.0f;   
			/** 現在表示中のゲージ割合（Lerp後） */
			float displayLevelRatio_ = 0.0f;
			/** Lerpの速度（大きいほど速い） */
			float levelLerpSpeed_ = 3.0f;
			bool isInvincible_ = false;   
			bool isVisible_ = true;
			/** HP全回復の表示をフレームをまたいで強制 */
			bool isHealPending_ = false;
			/** 回復アニメーション中フラグ */
			bool isHealAnimating_ = false;
			/** 回復アニメーション経過時間 */
			float healAnimTimer_ = 0.0f;
			/** アニメーション総時間 */
			float healAnimDuration_ = 1.5f;
			/** アニメーション開始時のHP割合 */
			float healStartRatio_ = 0.0f;
			/** 表示用HP割合 */
			float displayHpRatio_ = 0.0f;
			/** 目標HP割合 */
			float targetHpRatio_ = 0.0f;

		public:
			PlayerHpUIObject();
			~PlayerHpUIObject();
		public:
			void Update() override;
			void Render(RenderContext& rc) override;

			// BattleCharacterから level を受け取って表示
			void SetLevel(int level) { level_ = min(level, 10);}
			void AddLevelUpGauge(int exp) { levelUpIndex_ = min(levelUpIndex_ + exp, 10); }

			bool IsLevelUp() const { return isLevelUpPending_; }
			void ClearLevelUp() { isLevelUpPending_ = false; }

			void SetLevelUpUIObject(LevelUpUIObject* obj) { levelUpUIObject_ = obj; }
			void SetPlayer(app::actor::BattleCharacter* player) { player_ = player; }

			void StartInvincible(float time)
			{
				isInvincible_ = true;
				invincibleTimer_ = time;
				blinkTimer_ = 0.0f;
				isVisible_ = true;
			}
		};




		/*************************************************/


		class EnemyHpUIObject : public HpUIObject, public app::actor::IPoolable
		{
		private:
			bool isDead_ = false;

			std::unique_ptr<app::ui::Layout> layout_;

			app::actor::BattleCharacter* player_ = nullptr;
			app::actor::StoneEventCharacter* stoneTarget_ = nullptr;
			app::actor::MushroomEventCharacter* mushroomTarget_ = nullptr;

			float curHpOffsetX_ = 0.0f;
			float dmgHpOffsetX_ = 0.0f;

			int hpIndex_ = 0;
			/** 表示フラグ */
			bool isVisible_ = false;
		public:
			EnemyHpUIObject();
			~EnemyHpUIObject();
		public:
			void Update() override;
			void Render(RenderContext& rc) override;

			void SetPlayer(app::actor::BattleCharacter* player)
			{
				player_ = player;
			}
			// どのEnemyに追従するか設定
			void SetTargetEnemy(app::actor::StoneEventCharacter* enemy)
			{
				stoneTarget_ = enemy;
			}
			void SetTargetEnemy(app::actor::MushroomEventCharacter* enemy)
			{
				mushroomTarget_ = enemy;
			}
			void ClearTarget()
			{
				stoneTarget_ = nullptr;
				mushroomTarget_ = nullptr;
				isDead_ = true;
			}
			/** プールから取り出す時に呼ぶ */
			void OnSpawn()
			{
				isDead_ = false;
				isVisible_ = false;
				hpIndex_ = 10; // コンストラクタと同じ初期値
				damagePosX_ = 1.0f;
				damageDelayTimer_ = 0.0f;
				curHpOffsetX_ = 0.0f; // Layoutから再取得するので後述
				dmgHpOffsetX_ = 0.0f;
				stoneTarget_ = nullptr;
				mushroomTarget_ = nullptr;
				player_ = nullptr;

				// Layoutのオフセット座標を再取得
				auto menu = layout_->GetMenu();
				auto enemyCurHP = menu->GetUI<UIIcon>(Hash32("enemyCurrentHP"));
				auto enemyDmgHP = menu->GetUI<UIIcon>(Hash32("enemyDamageHP"));
				if (enemyCurHP) curHpOffsetX_ = enemyCurHP->transform.localPosition.x;
				if (enemyDmgHP) dmgHpOffsetX_ = enemyDmgHP->transform.localPosition.x;
			}

			/** プールに返す時に呼ぶ */
			void OnRecycle()
			{
				isDead_ = true;   // Update()を即スキップさせる
				isVisible_ = false;
				stoneTarget_ = nullptr;
				mushroomTarget_ = nullptr;
				player_ = nullptr;
			}
		};




		/*************************************************/


		class LevelUpUIObject : public HpUIObject
		{
		private:
			bool isDead_ = false;

			std::unique_ptr<app::ui::Layout> layout_;

			app::actor::BattleCharacter* player_ = nullptr;
			app::actor::StoneEventCharacter* stoneTarget_ = nullptr;
			app::actor::MushroomEventCharacter* mushroomTarget_ = nullptr;

			float curHpOffsetX_ = 0.0f;
			float dmgHpOffsetX_ = 0.0f;
			/** 攻撃UPアニメーションタイマー */
			float atkAnimTimer_ = 0.0f;
			/** レベルUPアニメーションタイマー */
			float levelAnimTimer_ = 0.0f;
			/** 全回復アニメーションタイマー */
			float maxHpAnimTimer_ = 0.0f;
			/** 退場アニメーションタイマー */
			float exitAnimTimer_ = 0.0f;
			float exitLeftTimer_ = 0.0f;

			/** 全回復アニメーションしたか */
			bool isHpMaxAnimPlayed_ = false;
			/** 攻撃UPアニメーションしたか */
			bool isAtkAnimPlayed_ = false;
			/** レベルUPアニメーションしたか */
			bool isLevelAnimPlayed_ = false;
			/** 退場アニメーションしたか（右移動） */
			bool isExitRightPlayed_ = false;
			/** 退場アニメーションしたか（左移動） */
			bool isExitLeftPlayed_ = false;
			/** 攻撃力UPも再生するか */
			bool isAtkUpLevel_ = false;



			int hpIndex_ = 0;
			/** 表示フラグ */
			bool isVisible_ = false;
		public:
			LevelUpUIObject();
			~LevelUpUIObject();
		public:
			void Update() override;
			void Render(RenderContext& rc) override;

			void SetPlayer(app::actor::BattleCharacter* player)
			{
				player_ = player;
			}
			// どのEnemyに追従するか設定
			void SetTargetEnemy(app::actor::StoneEventCharacter* enemy)
			{
				stoneTarget_ = enemy;
			}
			void SetTargetEnemy(app::actor::MushroomEventCharacter* enemy)
			{
				mushroomTarget_ = enemy;
			}
			void ClearTarget()
			{
				stoneTarget_ = nullptr;
				mushroomTarget_ = nullptr;
				isDead_ = true;
			}
			void TriggerLevelUp(int newLevel)
			{
				isLevelUpPending_ = true;
				// 偶数レベルなら攻撃力UPも再生
				isAtkUpLevel_ = (newLevel % 2 == 0)||(newLevel == 1);

				// レベルアップ時にHPを全回復
				if (player_)
				{
					player_->GetStatus()->SetCurrentHp(player_->GetStatus()->GetMaxHp());
				}

				atkAnimTimer_ = 0.0f;
				levelAnimTimer_ = 0.0f;
				exitAnimTimer_ = 0.0f;
				exitLeftTimer_ = 0.0f;
				isAtkAnimPlayed_ = false;
				isLevelAnimPlayed_ = false;
				isExitRightPlayed_ = false;
				isExitLeftPlayed_ = false;
				isHpMaxAnimPlayed_ = false;
				maxHpAnimTimer_ = 0.0f;

				// 攻撃力UPが不要なら最初からスキップ済み扱い
				isAtkAnimPlayed_ = !isAtkUpLevel_;
			}
		};
	}
}