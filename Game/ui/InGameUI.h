#pragma once
#include "Layout.h"
#include "actor/BattleCharacter.h"
#include "actor/EventCharacter.h"

namespace app
{
	namespace ui
	{
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
			std::unique_ptr<app::ui::Layout> layout_;
			/** 表示用のコピー */
			//int level_ = 0;

		public:
			PlayerHpUIObject();
			~PlayerHpUIObject();
		public:
			void Update() override;
			void Render(RenderContext& rc) override;

			// BattleCharacterから level を受け取って表示
			void SetLevel(int level) { level_ = min(level, 10);}
			void AddLevelUpGauge() { levelUpIndex_; }

			bool IsLevelUp() const { return isLevelUpPending_; }
			void ClearLevelUp() { isLevelUpPending_ = false; }
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
	}
}