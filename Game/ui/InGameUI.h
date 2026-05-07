#pragma once
#include "Layout.h"
#include "actor/BattleCharacter.h"
#include "actor/EventCharacter.h"

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
			bool isInvincible_ = false;   
			bool isVisible_ = true;       
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


		class EnemyHpUIObject : public HpUIObject
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
			/** 退場アニメーションタイマー */
			float exitAnimTimer_ = 0.0f;
			float exitLeftTimer_ = 0.0f;

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
				atkAnimTimer_ = 0.0f;
				levelAnimTimer_ = 0.0f;
				exitAnimTimer_ = 0.0f;
				exitLeftTimer_ = 0.0f;
				isAtkAnimPlayed_ = false;
				isLevelAnimPlayed_ = false;
				isExitRightPlayed_ = false;
				isExitLeftPlayed_ = false;

				// 攻撃力UPが不要なら最初からスキップ済み扱い
				isAtkAnimPlayed_ = !isAtkUpLevel_;
			}
		};
	}
}