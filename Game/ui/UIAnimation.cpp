/**
 * UIAnimation.cpp
 * UI用のアニメーション処理群
 */
#include "stdafx.h"
#include "UIAnimation.h"
#include "UIParts.h"


namespace app
{
	namespace ui
	{


		UIColorAnimation::UIColorAnimation()
		{
			SetFunc([&](Vector4 v)
				{
					ui_->color = v;
				});
		}




		/*******************************************************/


		UIScaleAnimation::UIScaleAnimation()
		{
			SetFunc([&](Vector3 s)
				{
					ui_->transform.localScale = s;
				});
		}




		/*******************************************************/


		UITranslateAniamtion::UITranslateAniamtion()
		{
			SetFunc([&](Vector3 s)
				{
					ui_->transform.localPosition = s;
				});
		}




		/*******************************************************/


		UITranslateOffsetAnimation::UITranslateOffsetAnimation()
		{
			SetFunc([&](Vector3 offset)
				{
					ui_->transform.localPosition.Add(offset);
				});
		}




		/*******************************************************/


		UIRotationAnimation::UIRotationAnimation()
		{
			SetFunc([&](float s)
				{
					ui_->transform.localRotation.SetRotationDegZ(s);
				});
		}




		/*******************************************************/


		void UIAnimationSequence::Update(float deltaTime)
		{
			if (!isPlaying_ || !target_) return;

			// ディレイ待ち
			if (waitingDelay_) {
				delayTimer_ -= deltaTime;
				if (delayTimer_ > 0.0f) return;
				waitingDelay_ = false;
				StartCurrentStep();
				return;
			}

			// 現在のアニメーション完了チェック
			if (currentIndex_ >= 0 && currentIndex_ < static_cast<int>(steps_.size())) {
				const auto& step = steps_[currentIndex_];
				UIAnimationBase* anim = target_->FindAnimation(step.animationKey);
				if (anim && !anim->IsPlay()) {
					// 完了コールバック
					if (step.onComplete) step.onComplete();
					AdvanceToNext();
				}
			}
		}


		void UIAnimationSequence::StartCurrentStep()
		{
			if (currentIndex_ < 0 || currentIndex_ >= static_cast<int>(steps_.size())) return;

			const auto& step = steps_[currentIndex_];
			UIAnimationBase* anim = target_->FindAnimation(step.animationKey);
			if (anim) {
				if (step.onStart) step.onStart();
				anim->Play();
			}
			else {
				// アニメーションが見つからない場合はスキップ
				AdvanceToNext();
			}
		}
	}
}