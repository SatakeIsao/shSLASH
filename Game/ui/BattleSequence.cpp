#include "stdafx.h"
#include "BattleSequence.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"

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
			icon->color = Vector4::White;
			app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(icon, Hash32("CountDown_ScaleIn"));
			auto* anim = icon->FindAnimation(Hash32("CountDown_ScaleIn"));
			if (anim) anim->Play();
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

		void BattleSequence::PlayTimeUp()
		{
			if (currentDown_ == SequenceName::TimeUp) return;
			currentDown_ = SequenceName::TimeUp;

			auto* menu = layout_->GetMenu();
			if (!menu) return;

			auto* icon = menu->GetUI<UIIcon>(Hash32("TimeUp"));
			if (!icon) return;

			icon->color = Vector4::White;
			icon->transform.localPosition = Vector3(1000.0f, 0.0f, 0.0f);

			app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(icon, Hash32("TimeUpSlideIn"));
			app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(icon, Hash32("TimeUpSlideBack"));

			timeUpHoldTimer_ = kTimeUpHoldDuration;

			timeUpSequence_.Clear();
			timeUpSequence_.Add(Hash32("TimeUpSlideIn"))
			               .Add(Hash32("TimeUpSlideBack"));
			timeUpSequence_.Play(icon);
		}

		void BattleSequence::Update()
		{
			layout_->Update();
			auto* menu = layout_->GetMenu();
			if (!menu) return;

			const float dt = g_gameTime->GetFrameDeltaTime();

			if (currentDown_ == SequenceName::TimeUp)
			{
				timeUpSequence_.Update(dt);
				if (!timeUpSequence_.IsPlaying() && timeUpHoldTimer_ > 0.0f)
				{
					timeUpHoldTimer_ -= dt;
					if (timeUpHoldTimer_ < 0.0f) timeUpHoldTimer_ = 0.0f;
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
			auto* count3Icon = menu->GetUI<UIIcon>(Hash32("Count3"));
			auto* count2Icon = menu->GetUI<UIIcon>(Hash32("Count2"));
			auto* count1Icon = menu->GetUI<UIIcon>(Hash32("Count1"));
			auto* startIcon  = menu->GetUI<UIIcon>(Hash32("Start"));
			auto* timeUpIcon = menu->GetUI<UIIcon>(Hash32("TimeUp"));

			if (count3Icon) count3Icon->isDraw = (currentDown_ == SequenceName::Count3);
			if (count2Icon) count2Icon->isDraw = (currentDown_ == SequenceName::Count2);
			if (count1Icon) count1Icon->isDraw = (currentDown_ == SequenceName::Count1);
			if (startIcon)  startIcon->isDraw  = (currentDown_ == SequenceName::Start);
			if (timeUpIcon) timeUpIcon->isDraw  = (currentDown_ == SequenceName::TimeUp);
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
