#pragma once
#include "Layout.h"

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
		private:
			std::unique_ptr<app::ui::Layout> layout_;
			static constexpr float kDamageDelayTime = 0.5f;
			float damageDelayTimer_ = 0.0f;
			//ディレイHPバーがHPバーに線形補間で追従する時用
			float lerpVal_ = 0.5f;
			float damagePosX_ = 0.0f;
			float damageScaleX_ = 0.0f;
			int index_ = 0;
			int levelUpIndex_ = 0;
			int level_ = 0;

			bool isCounting_ = true;


		public:
			HpUIObject();
			~HpUIObject();
		public:
			void Update();
			void Render(RenderContext& rc);
		};
	}
}