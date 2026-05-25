#include "stdafx.h"
#include "PauseMenu.h"
#include "battle/BattleManager.h"
#include "core/ParameterManager.h"
#include "sound/SoundManager.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"

namespace app
{
	namespace ui
	{
		PauseMenu::PauseMenu()
		{
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterPauseMenuParameter>("Assets/master/PauseMenuParameter.json", [](const nlohmann::json& j, app::core::MasterPauseMenuParameter& p)
				{
					p.cursolPositionX[0] = j["cursolPositionXA"];
					p.cursolPositionX[1] = j["cursolPositionXB"];
					p.cursolPositionY[0] = j["cursolPositionYA"];
					p.cursolPositionY[1] = j["cursolPositionYB"];
				});
		}

		PauseMenu::~PauseMenu()
		{
			app::core::ParameterManager::Get().UnloadParameter<app::core::MasterPauseMenuParameter>();
		}

		void PauseMenu::Update()
		{
			auto* canvas = GetCanvas();
			bool isAnimating = false;

			if (canvas) {
				auto* inAnim = canvas->FindAnimation(Hash32("SlideIn_Pause_Overshoot"));
				auto* backAnim = canvas->FindAnimation(Hash32("SlideBack_Pause"));
				auto* outAnim = canvas->FindAnimation(Hash32("SlideOut_Pause_Exit"));

				if (inAnim && !inAnim->IsPlay()) {
					canvas->RemoveAnimation(Hash32("SlideIn_Pause_Overshoot"));

					app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(canvas, Hash32("SlideBack_Pause"));
					auto* newBackAnim = canvas->FindAnimation(Hash32("SlideBack_Pause"));
					if (newBackAnim) newBackAnim->Play();
				}

				// アニメーション中かどうかのフラグを立てる
				inAnim = canvas->FindAnimation(Hash32("SlideIn_Pause_Overshoot"));
				backAnim = canvas->FindAnimation(Hash32("SlideBack_Pause"));
				if ((inAnim && inAnim->IsPlay()) || (backAnim && backAnim->IsPlay()) || (outAnim && outAnim->IsPlay())) {
					isAnimating = true;
				}
			}

			if (!isAnimating)
			{
				if (g_pad[0]->IsTrigger(enButtonDown))
				{
					app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
					cursolIndex_++;
					if (cursolIndex_ >= 1) cursolIndex_ = 1;
				}
				if (g_pad[0]->IsTrigger(enButtonUp))
				{
					app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
					cursolIndex_--;
					if (cursolIndex_ < 0) cursolIndex_ = 0;
				}

				// カーソルの位置更新
				auto* parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterPauseMenuParameter>();
				{
					const float x = parameter->cursolPositionX[cursolIndex_];
					const float y = parameter->cursolPositionY[cursolIndex_];
					auto cursol = GetUI<UIIcon>(Hash32("Cursol"));
					if (cursol) {
						cursol->transform.localPosition.x = x;
						cursol->transform.localPosition.y = y;
					}
				}
			}

			MenuBase::Update();

			if (canvas) {
				auto* backFilter = GetUI<app::ui::UIIcon>(Hash32("BackFilter"));
				if (backFilter) {
					backFilter->transform.localPosition.x = -canvas->transform.localPosition.x;
					backFilter->transform.localPosition.y = -canvas->transform.localPosition.y;
					backFilter->Update(); 
				}
			}
		}

		void PauseMenu::OnOpen()
		{
			auto* canvas = GetCanvas();
			if (canvas)
			{
				canvas->transform.localScale = Vector3::One;
				canvas->RemoveAnimation(Hash32("SlideOut_Pause_Exit"));
				canvas->RemoveAnimation(Hash32("SlideBack_Pause"));

				app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(canvas, Hash32("SlideIn_Pause_Overshoot"));
				auto* anim = canvas->FindAnimation(Hash32("SlideIn_Pause_Overshoot"));
				if (anim) anim->Play();
			}

			auto* backFilter = GetUI<app::ui::UIIcon>(Hash32("BackFilter"));
			if (backFilter) {
				backFilter->isDraw = true;
			}

			auto* cursol = GetUI<app::ui::UIIcon>(Hash32("Cursol"));
			if (cursol) {
				cursol->isDraw = true;
				auto* anim = cursol->FindAnimation(Hash32("FadeIn"));
				if (anim) anim->Play();
			}
		}

		void PauseMenu::OnClose()
		{
			auto* canvas = GetCanvas();
			if (canvas)
			{
				canvas->RemoveAnimation(Hash32("SlideIn_Pause_Overshoot"));
				canvas->RemoveAnimation(Hash32("SlideBack_Pause"));

				app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(canvas, Hash32("SlideOut_Pause_Exit"));
				auto* anim = canvas->FindAnimation(Hash32("SlideOut_Pause_Exit"));
				if (anim) anim->Play();
			}

			auto* backFilter = GetUI<app::ui::UIIcon>(Hash32("BackFilter"));
			if (backFilter) {
				backFilter->isDraw = false;
			}

			auto* cursol = GetUI<app::ui::UIIcon>(Hash32("Cursol"));
			if (cursol) {
				cursol->isDraw = false;
				cursol->StopSpriteAnimation();
			}
		}

		bool PauseMenu::IsPause()
		{
			auto* canvas = GetCanvas();
			if (canvas) {
				auto* anim = canvas->FindAnimation(Hash32("SlideOut_Pause_Exit"));
				if (anim && anim->IsPlay()) {
					return true;
				}
			}
			return false;
		}

		void PauseMenu::InitializeLogic()
		{
			auto* canvas = GetCanvas();
			if (canvas)
			{
				canvas->transform.localScale = Vector3::One;
			}


			auto* cursol = GetUI<app::ui::UIIcon>(Hash32("Cursol"));
			if (cursol) {
				app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(cursol, Hash32("FadeIn"));
				auto* anim = cursol->FindAnimation(Hash32("FadeIn"));
				if (anim) anim->Play();
			}
		}
	}
}