#include "stdafx.h"
#include "BattleSequence.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"
#include "sound/SoundManager.h"

namespace app
{
	namespace ui
	{
		BattleSequence::BattleSequence()
		{
			layout_ = std::make_unique<app::ui::Layout>();
			layout_->Initialize<app::ui::MenuBase>("Assets/ui/layout/BattleSequenceMenuLayout.json");
			currentDown_ = SequenceName::Wait;
		}

		BattleSequence::~BattleSequence() {}

		void BattleSequence::ShowCountIcon(UIIcon* icon)
		{
			if (!icon) return;
			icon->color = Vector4(1.0f, 1.0f, 1.0f, 0.0f);

			app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(icon, Hash32("CountDown_ScaleIn"));
			auto* scaleAnim = icon->FindAnimation(Hash32("CountDown_ScaleIn"));
			if (scaleAnim) scaleAnim->Play();

			app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(icon, Hash32("CountDown_FlashIn"));
			auto* flashAnim = icon->FindAnimation(Hash32("CountDown_FlashIn"));
			if (flashAnim) flashAnim->Play();

			fadeOutStarted_ = false;
		}

		void BattleSequence::ShowStartIcon(UIIcon* icon)
		{
			if (!icon) return;
			icon->color = Vector4::White;
			app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(icon, Hash32("Start_ScaleIn"));
			auto* anim = icon->FindAnimation(Hash32("Start_ScaleIn"));
			if (anim) anim->Play();
			fadeOutStarted_ = false;
		}

		void BattleSequence::ShowStartBlurIcon(UIIcon* icon)
		{
			if (!icon) return;
			icon->color = Vector4::White;
			app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(icon, Hash32("StartBlur_ScaleExpand"));
			auto* scaleAnim = icon->FindAnimation(Hash32("StartBlur_ScaleExpand"));
			if (scaleAnim) scaleAnim->Play();
			app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(icon, Hash32("StartBlur_FadeOut"));
			auto* fadeAnim = icon->FindAnimation(Hash32("StartBlur_FadeOut"));
			if (fadeAnim) fadeAnim->Play();
		}

		void BattleSequence::TryStartFadeOut(UIIcon* icon, bool isStart)
		{
			if (!icon || fadeOutStarted_) return;
			fadeOutStarted_ = true;

			if (isStart)
			{
				icon->RemoveAnimation(Hash32("Start_ScaleIn"));

				app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(icon, Hash32("Start_ScaleExpand"));
				auto* scaleAnim = icon->FindAnimation(Hash32("Start_ScaleExpand"));
				if (scaleAnim) scaleAnim->Play();

				app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(icon, Hash32("Start_FadeOut"));
				auto* fadeAnim = icon->FindAnimation(Hash32("Start_FadeOut"));
				if (fadeAnim) fadeAnim->Play();
			}
			else
			{
				icon->RemoveAnimation(Hash32("CountDown_ScaleIn"));

				app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(icon, Hash32("CountDown_ScaleOut"));
				auto* scaleAnim = icon->FindAnimation(Hash32("CountDown_ScaleOut"));
				if (scaleAnim) scaleAnim->Play();

				app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(icon, Hash32("CountDown_FadeOut"));
				auto* fadeAnim = icon->FindAnimation(Hash32("CountDown_FadeOut"));
				if (fadeAnim) fadeAnim->Play();
			}
		}

		static void StartTimeUpSideIcon(UIIcon* icon, uint32_t slideKey)
		{
			if (!icon) return;
			// alpha 0.5 で開始し、スライド中にフェードアウト（アニメが 0.5→0 を駆動）
			icon->color = Vector4(1.0f, 1.0f, 1.0f, 0.5f);

			app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(icon, Hash32("TimeUpSideFadeIn"));
			auto* fadeOut = icon->FindAnimation(Hash32("TimeUpSideFadeIn"));
			if (fadeOut) fadeOut->Play();

			app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(icon, slideKey);
		}

		void BattleSequence::ShowTimeUpCountdown(int count)
		{
			if (count < 1 || count > 5) return;
			auto* menu = layout_->GetMenu();
			if (!menu) return;

			// 前の数字を即座に非表示
			if (timeUpCountActiveIcon_)
			{
				timeUpCountActiveIcon_->isDraw = false;
				timeUpCountActiveIcon_->color  = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
				timeUpCountActiveIcon_ = nullptr;
			}

			static const uint32_t kKeys[5] = {
				Hash32("TimeUpCount1"), Hash32("TimeUpCount2"), Hash32("TimeUpCount3"),
				Hash32("TimeUpCount4"), Hash32("TimeUpCount5")
			};
			timeUpCountActiveIcon_ = menu->GetUI<UIIcon>(kKeys[count - 1]);
			if (!timeUpCountActiveIcon_) return;

			timeUpCountActiveIcon_->isDraw = true;
			timeUpCountActiveIcon_->color  = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
			timeUpCountActiveIcon_->transform.localScale = Vector3(1.0f, 1.0f, 1.0f);

			app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(timeUpCountActiveIcon_, Hash32("TimeUpCount_ScaleIn"));
			auto* scaleIn = timeUpCountActiveIcon_->FindAnimation(Hash32("TimeUpCount_ScaleIn"));
			if (scaleIn) scaleIn->Play();

			app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(timeUpCountActiveIcon_, Hash32("TimeUpCount_FadeIn"));
			auto* fadeIn = timeUpCountActiveIcon_->FindAnimation(Hash32("TimeUpCount_FadeIn"));
			if (fadeIn) fadeIn->Play();

			timeUpCountTimer_       = kTimeUpCountHoldDuration + kTimeUpCountFadeDuration;
			timeUpCountFadeStarted_ = false;

			static const app::SoundKind kCountVoices[5] = {
				app::SoundKind::CountdownVoice1,
				app::SoundKind::CountdownVoice2,
				app::SoundKind::CountdownVoice3,
				app::SoundKind::CountdownVoice4,
				app::SoundKind::CountdownVoice5,
			};
			app::SoundManager::Get().PlaySE(static_cast<int>(kCountVoices[count - 1]));
		}

		void BattleSequence::PlayTimeUp()
		{
			if (currentDown_ == SequenceName::TimeUp) return;
			currentDown_ = SequenceName::TimeUp;

			auto* menu = layout_->GetMenu();
			if (!menu) return;

			auto* iconLeft   = menu->GetUI<UIIcon>(Hash32("TimeUpLeft"));
			auto* iconRight  = menu->GetUI<UIIcon>(Hash32("TimeUpRight"));
			auto* iconCenter = menu->GetUI<UIIcon>(Hash32("TimeUp"));

			// 中央アイコンは収束まで非表示
			if (iconCenter) iconCenter->color = Vector4(1.0f, 1.0f, 1.0f, 0.0f);

			// 左右アイコン：半透明で両端から高速スライド
			StartTimeUpSideIcon(iconLeft,  Hash32("TimeUpLeftSlideIn"));
			StartTimeUpSideIcon(iconRight, Hash32("TimeUpRightSlideIn"));

			timeUpLeftSequence_.Clear();
			timeUpLeftSequence_.Add(Hash32("TimeUpLeftSlideIn"));
			timeUpLeftSequence_.Play(iconLeft);

			timeUpRightSequence_.Clear();
			timeUpRightSequence_.Add(Hash32("TimeUpRightSlideIn"));
			timeUpRightSequence_.Play(iconRight);

			timeUpConvergeTimer_ = kTimeUpConvergeDuration;
			timeUpPhase_         = TimeUpPhase::Converging;
			timeUpHoldTimer_     = 0.0f;

			app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::CountdownVoiceTimeUp));
		}

		static void TriggerTimeUpConvergeImpact(UIIcon* iconCenter, UIIcon* iconFlash, UIAnimationSequence& shakeSeq)
		{
			if (!iconCenter) return;
			iconCenter->color = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
			iconCenter->transform.localPosition = Vector3::Zero;

			app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(iconCenter, Hash32("TimeUpCenterFlash"));
			auto* flashAnim = iconCenter->FindAnimation(Hash32("TimeUpCenterFlash"));
			if (flashAnim) flashAnim->Play();

			app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(iconCenter, Hash32("TimeUpCenterPop"));
			auto* popAnim = iconCenter->FindAnimation(Hash32("TimeUpCenterPop"));
			if (popAnim) popAnim->Play();

			// ロゴ上の白フラッシュ：瞬間点灯→フェードアウト
			if (iconFlash)
			{
				iconFlash->color = Vector4(1.0f, 1.0f, 1.0f, 0.9f);
				app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(iconFlash, Hash32("TimeUpFlashFade"));
				auto* whiteFade = iconFlash->FindAnimation(Hash32("TimeUpFlashFade"));
				if (whiteFade) whiteFade->Play();
			}

			// 上下2サイクルの減衰揺れ（タイトル斬撃と同スタイル）
			app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(iconCenter, Hash32("TimeUpCenterShake_1"));
			app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(iconCenter, Hash32("TimeUpCenterShake_2"));
			app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(iconCenter, Hash32("TimeUpCenterShake_3"));
			app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(iconCenter, Hash32("TimeUpCenterShake_4"));

			shakeSeq.Clear();
			shakeSeq.Add(Hash32("TimeUpCenterShake_1"))
			        .Add(Hash32("TimeUpCenterShake_2"))
			        .Add(Hash32("TimeUpCenterShake_3"))
			        .Add(Hash32("TimeUpCenterShake_4"));
			shakeSeq.Play(iconCenter);
		}

		void BattleSequence::DebugToggleTimeUp()
		{
			debugTimeUpVisible_ = !debugTimeUpVisible_;

			auto* menu = layout_->GetMenu();
			if (!menu) return;

			auto* iconLeft   = menu->GetUI<UIIcon>(Hash32("TimeUpLeft"));
			auto* iconRight  = menu->GetUI<UIIcon>(Hash32("TimeUpRight"));
			auto* iconCenter = menu->GetUI<UIIcon>(Hash32("TimeUp"));
			auto* iconFlash  = menu->GetUI<UIIcon>(Hash32("TimeUpFlash"));

			if (debugTimeUpVisible_)
			{
				if (iconCenter) iconCenter->color = Vector4(1.0f, 1.0f, 1.0f, 0.0f);

				StartTimeUpSideIcon(iconLeft,  Hash32("TimeUpLeftSlideIn"));
				StartTimeUpSideIcon(iconRight, Hash32("TimeUpRightSlideIn"));

				timeUpLeftSequence_.Clear();
				timeUpLeftSequence_.Add(Hash32("TimeUpLeftSlideIn"));
				timeUpLeftSequence_.Play(iconLeft);

				timeUpRightSequence_.Clear();
				timeUpRightSequence_.Add(Hash32("TimeUpRightSlideIn"));
				timeUpRightSequence_.Play(iconRight);

				timeUpConvergeTimer_ = kTimeUpConvergeDuration;
				timeUpPhase_         = TimeUpPhase::Converging;
				timeUpHoldTimer_     = 0.0f;
			}
			else
			{
				timeUpLeftSequence_.Clear();
				timeUpRightSequence_.Clear();
				timeUpCenterShakeSequence_.Clear();
				timeUpPhase_         = TimeUpPhase::None;
				timeUpHoldTimer_     = 0.0f;
				timeUpConvergeTimer_ = 0.0f;

				auto hideIcon = [](UIIcon* icon) {
					if (!icon) return;
					icon->color = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
					icon->transform.localPosition = Vector3::Zero;
				};
				hideIcon(iconLeft);
				hideIcon(iconRight);
				hideIcon(iconCenter);
				hideIcon(iconFlash);
			}
		}

		void BattleSequence::Update()
		{
			layout_->Update();
			auto* menu = layout_->GetMenu();
			if (!menu) return;

			const float dt = g_gameTime->GetFrameDeltaTime();

			if (currentDown_ == SequenceName::TimeUp || debugTimeUpVisible_)
			{
				if (timeUpPhase_ == TimeUpPhase::Converging)
				{
					timeUpLeftSequence_.Update(dt);
					timeUpRightSequence_.Update(dt);

					timeUpConvergeTimer_ -= dt;
					if (timeUpConvergeTimer_ <= 0.0f)
					{
						auto* iconCenter = menu->GetUI<UIIcon>(Hash32("TimeUp"));
						auto* iconFlash  = menu->GetUI<UIIcon>(Hash32("TimeUpFlash"));
						TriggerTimeUpConvergeImpact(iconCenter, iconFlash, timeUpCenterShakeSequence_);

						timeUpPhase_     = TimeUpPhase::Holding;
						timeUpHoldTimer_ = kTimeUpHoldDuration;
					}
				}
				else if (timeUpPhase_ == TimeUpPhase::Holding)
				{
					timeUpCenterShakeSequence_.Update(dt);

					if (timeUpHoldTimer_ > 0.0f)
					{
						timeUpHoldTimer_ -= dt;
						if (timeUpHoldTimer_ < 0.0f) timeUpHoldTimer_ = 0.0f;
					}
				}
			}

			// タイムアップ前カウントダウン（5〜1）の退場管理
			if (timeUpCountActiveIcon_ && timeUpCountTimer_ >= 0.0f)
			{
				timeUpCountTimer_ -= dt;
				if (!timeUpCountFadeStarted_ && timeUpCountTimer_ <= kTimeUpCountFadeDuration)
				{
					timeUpCountFadeStarted_ = true;
					app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(timeUpCountActiveIcon_, Hash32("TimeUpCount_ScaleOut"));
					auto* scaleOut = timeUpCountActiveIcon_->FindAnimation(Hash32("TimeUpCount_ScaleOut"));
					if (scaleOut) scaleOut->Play();
					app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(timeUpCountActiveIcon_, Hash32("TimeUpCount_FadeOut"));
					auto* fadeOut = timeUpCountActiveIcon_->FindAnimation(Hash32("TimeUpCount_FadeOut"));
					if (fadeOut) fadeOut->Play();
				}
				if (timeUpCountTimer_ <= 0.0f)
				{
					timeUpCountActiveIcon_->isDraw  = false;
					timeUpCountActiveIcon_->color   = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
					timeUpCountActiveIcon_          = nullptr;
					timeUpCountTimer_               = -1.0f;
					timeUpCountFadeStarted_         = false;
				}
			}

			switch (currentDown_)
			{
			case SequenceName::Wait:
				delayTimer_ -= dt;
				if (delayTimer_ <= 0.0f)
				{
					currentDown_ = SequenceName::Count3;
					countTimer_ = kCountDuration;
					ShowCountIcon(menu->GetUI<UIIcon>(Hash32("Count3")));
					app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::CountdownVoice3));
				}
				break;

			case SequenceName::Count3:
				countTimer_ -= dt;
				if (countTimer_ <= kFadeOutDelay)
				{
					TryStartFadeOut(menu->GetUI<UIIcon>(Hash32("Count3")), false);
				}
				if (countTimer_ <= 0.0f)
				{
					currentDown_ = SequenceName::Count2;
					countTimer_ = kCountDuration;
					fadeOutStarted_ = false;
					ShowCountIcon(menu->GetUI<UIIcon>(Hash32("Count2")));
					app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::CountdownVoice2));
				}
				break;

			case SequenceName::Count2:
				countTimer_ -= dt;
				if (countTimer_ <= kFadeOutDelay)
				{
					TryStartFadeOut(menu->GetUI<UIIcon>(Hash32("Count2")), false);
				}
				if (countTimer_ <= 0.0f)
				{
					currentDown_ = SequenceName::Count1;
					countTimer_ = kCountDuration;
					fadeOutStarted_ = false;
					ShowCountIcon(menu->GetUI<UIIcon>(Hash32("Count1")));
					app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::CountdownVoice1));
				}
				break;

			case SequenceName::Count1:
				countTimer_ -= dt;
				if (countTimer_ <= kFadeOutDelay)
				{
					TryStartFadeOut(menu->GetUI<UIIcon>(Hash32("Count1")), false);
				}
				if (countTimer_ <= 0.0f)
				{
					currentDown_ = SequenceName::Start;
					countTimer_ = kStartDuration;
					fadeOutStarted_ = false;
					ShowStartIcon(menu->GetUI<UIIcon>(Hash32("Start")));
					ShowStartBlurIcon(menu->GetUI<UIIcon>(Hash32("StartBlur")));
					app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::CountdownVoiceStart));
				}
				break;

			case SequenceName::Start:
				countTimer_ -= dt;
				if (countTimer_ <= kStartFadeOutDelay)
				{
					TryStartFadeOut(menu->GetUI<UIIcon>(Hash32("Start")), true);
				}
				if (countTimer_ <= 0.0f)
				{
					currentDown_ = SequenceName::Finished;
				}
				break;

			default:
				break;
			}

			// isDraw を毎フレーム現在の状態から設定する
			// (hot-reload でアイコンが再生成されても常に正しい表示になる)
			auto* count3Icon      = menu->GetUI<UIIcon>(Hash32("Count3"));
			auto* count2Icon      = menu->GetUI<UIIcon>(Hash32("Count2"));
			auto* count1Icon      = menu->GetUI<UIIcon>(Hash32("Count1"));
			auto* startIcon       = menu->GetUI<UIIcon>(Hash32("Start"));
			auto* startBlurIcon   = menu->GetUI<UIIcon>(Hash32("StartBlur"));
			auto* timeUpLeftIcon  = menu->GetUI<UIIcon>(Hash32("TimeUpLeft"));
			auto* timeUpRightIcon = menu->GetUI<UIIcon>(Hash32("TimeUpRight"));
			auto* timeUpIcon      = menu->GetUI<UIIcon>(Hash32("TimeUp"));
			auto* timeUpFlashIcon = menu->GetUI<UIIcon>(Hash32("TimeUpFlash"));

			const bool isTimeUpActive = (currentDown_ == SequenceName::TimeUp) || debugTimeUpVisible_;

			if (count3Icon)      count3Icon->isDraw      = (currentDown_ == SequenceName::Count3);
			if (count2Icon)      count2Icon->isDraw      = (currentDown_ == SequenceName::Count2);
			if (count1Icon)      count1Icon->isDraw      = (currentDown_ == SequenceName::Count1);
			if (startIcon)       startIcon->isDraw       = (currentDown_ == SequenceName::Start);
			if (startBlurIcon)   startBlurIcon->isDraw   = (currentDown_ == SequenceName::Start);
			if (timeUpLeftIcon)  timeUpLeftIcon->isDraw  = isTimeUpActive;
			if (timeUpRightIcon) timeUpRightIcon->isDraw = isTimeUpActive;
			if (timeUpIcon)      timeUpIcon->isDraw      = isTimeUpActive;
			if (timeUpFlashIcon) timeUpFlashIcon->isDraw = isTimeUpActive;
		}

		void BattleSequence::Render(RenderContext& rc)
		{
			if (layout_)
			{
				layout_->Render(rc);
			}
		}
	}
}
