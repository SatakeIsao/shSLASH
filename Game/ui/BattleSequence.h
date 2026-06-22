/**
 * BattleSequence.h
 * バトルシーケンス
 */

#pragma once
#include "Layout.h"
#include "UIParts.h"

namespace app
{
	namespace ui
	{
		class BattleSequence : public IGameObject
		{
			enum class SequenceName {
				Wait,
				Count3,
				Count2,
				Count1,
				Start,
				TimeUp,
				Finished
			};

		private:
			std::unique_ptr<app::ui::Layout> layout_;
			SequenceName currentDown_ = SequenceName::Wait;

			float delayTimer_ = 1.5f;
			float countTimer_ = 0.0f;
			float timeUpHoldTimer_ = 0.0f;
			float timeUpConvergeTimer_ = 0.0f;
			bool fadeOutStarted_ = false;
			bool debugTimeUpVisible_ = false;

			enum class TimeUpPhase { None, Converging, Holding };
			TimeUpPhase timeUpPhase_ = TimeUpPhase::None;

			UIAnimationSequence timeUpLeftSequence_;
			UIAnimationSequence timeUpRightSequence_;
			UIAnimationSequence timeUpCenterShakeSequence_;

			UIIcon* timeUpCountActiveIcon_    = nullptr;
			float   timeUpCountTimer_         = -1.0f;
			bool    timeUpCountFadeStarted_   = false;

			static constexpr float kCountDuration        = 0.85f;
			static constexpr float kFadeOutDelay         = 0.35f;
			static constexpr float kStartDuration        = 1.2f;
			static constexpr float kStartFadeOutDelay    = 0.5f;
			static constexpr float kTimeUpHoldDuration   = 1.5f;
			static constexpr float kTimeUpConvergeDuration  = 0.16f;
			static constexpr float kTimeUpCountHoldDuration = 0.60f;
			static constexpr float kTimeUpCountFadeDuration = 0.20f;

		private:
			void ShowCountIcon(UIIcon* icon);
			void ShowStartIcon(UIIcon* icon);
			void ShowStartBlurIcon(UIIcon* icon);
			void TryStartFadeOut(UIIcon* icon, bool isStart);

		public:
			BattleSequence();
			~BattleSequence();
		public:
			void Update() override;
			void Render(RenderContext& rc);

		public:
			bool IsPlaying() const {
				return currentDown_ != SequenceName::Wait && currentDown_ != SequenceName::Finished;
			}
			bool IsFinished() const {
				return currentDown_ == SequenceName::Finished;
			}
			bool IsTimeUpFinished() const {
				return currentDown_ == SequenceName::TimeUp
					&& timeUpPhase_ == TimeUpPhase::Holding
					&& timeUpHoldTimer_ <= 0.0f;
			}
			bool IsOpeningDone() const {
				return currentDown_ == SequenceName::Finished || currentDown_ == SequenceName::TimeUp;
			}
			void PlayTimeUp();
			void DebugToggleTimeUp();
			void ShowTimeUpCountdown(int count);
		};
	}
}
