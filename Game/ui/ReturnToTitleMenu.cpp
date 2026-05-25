#include "stdafx.h"
#include "ReturnToTitleMenu.h"
#include "core/ParameterManager.h"
#include "sound/SoundManager.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"


namespace app
{
	namespace ui
	{
		ReturnToTitleMenu::ReturnToTitleMenu()
		{
			app::core::ParameterManager::Get().LoadParameter<app::core::ReturnToTitleMenuParameter>("Assets/master/ReturnToTitleMenuParameter.json", [](const nlohmann::json& j, app::core::ReturnToTitleMenuParameter& p)
				{
					//TODO: X座標もやりたいなぁ
					p.cursolPositionX[0] = j["cursolPositionXA"];
					p.cursolPositionX[1] = j["cursolPositionXB"];

					p.cursolPositionY[0] = j["cursolPositionYA"];
					p.cursolPositionY[1] = j["cursolPositionYB"];
				});
		}

		ReturnToTitleMenu:: ~ReturnToTitleMenu()
		{
			app::core::ParameterManager::Get().UnloadParameter<app::core::ReturnToTitleMenuParameter>();
		}

		void ReturnToTitleMenu::Update()
		{
			auto* canvas = GetCanvas();
			if (canvas)
			{
				//閉じる
				{
					auto* closeAnim = canvas->FindAnimation(Hash32("ScaleDown"));
					if (closeAnim && !closeAnim->IsPlay())
					{
						canvas->RemoveAnimation(Hash32("ScaleDown"));
						closeAnim = nullptr;
						isPause_ = false;
					}
				}
				//開く
				{
					auto* openAnim = canvas->FindAnimation(Hash32("ScaleUp"));
					if (openAnim && !openAnim->IsPlay())
					{
						canvas->RemoveAnimation(Hash32("ScaleUp"));
					}
				}
			}

			if (g_pad[0]->IsTrigger(enButtonDown))
			{
				app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
				cursolIndex_++;
				if (cursolIndex_ >= 1)
				{
					cursolIndex_ = 1;
				}
			}
			if (g_pad[0]->IsTrigger(enButtonUp))
			{
				app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
				cursolIndex_--;
				if (cursolIndex_ < 0)
				{
					cursolIndex_ = 0;
				}
			}
			/** はいをえらぶ */
			if (cursolIndex_ == 0
				&& g_pad[0]->IsTrigger(enButtonA))
			{
				app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
				isDecidedYes_ = true;
			}
			/** いいえをえらぶ */
			if (cursolIndex_ == 1
				&& g_pad[0]->IsTrigger(enButtonA))
			{
				app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
				isDecidedNo_ = true;
			}
			else
			{
				isDecidedNo_ = false;
			}

			// 動的に数値をUIに設定
			auto* parameter = app::core::ParameterManager::Get().GetParameter<app::core::ReturnToTitleMenuParameter>();
			// はい/いいえテキストの場所
			{
				const float x = parameter->cursolPositionX[cursolIndex_];
				auto cursol = GetUI<UIIcon>(Hash32("Cursol"));
				cursol->transform.localPosition.x = x;
			}
			{
				const float y = parameter->cursolPositionY[cursolIndex_];
				auto cursol = GetUI<UIIcon>(Hash32("Cursol"));
				cursol->transform.localPosition.y = y;
			}

			PlaySelectedAnimation();
			MenuBase::Update();
		}

		void ReturnToTitleMenu::OnOpen()
		{
			isPause_ = true;

			//キャンバス
			{
				auto* canvas = GetCanvas();
				if (canvas)
				{
					canvas->RemoveAnimation(Hash32("ScaleUp"));
					canvas->RemoveAnimation(Hash32("ScaleDown"));
					//アニメーションをアタッチ
					app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(canvas, Hash32("ScaleUp"));
					auto* openAnim = canvas->FindAnimation(Hash32("ScaleUp"));
					if (openAnim) openAnim->Play();
				}
			}
			auto* cursol = GetUI<app::ui::UIIcon>(Hash32("Cursol"));
			if (cursol) {
				cursol->isDraw = true;
				auto* anim = cursol->FindAnimation(Hash32("FadeIn"));
				if (anim) anim->Play();
			}
		}

		void ReturnToTitleMenu::OnClose()
		{
			// キャンバス
			{
				auto* canvas = GetCanvas();
				if (canvas)
				{
					canvas->RemoveAnimation(Hash32("ScaleUp"));
					canvas->RemoveAnimation(Hash32("ScaleDown"));
					//アニメーションをアタッチ
					app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(canvas, Hash32("ScaleDown"));
					auto* closeAnim = canvas->FindAnimation(Hash32("ScaleDown"));
					if (closeAnim) closeAnim->Play();
				}
			}
			auto* cursol = GetUI<app::ui::UIIcon>(Hash32("Cursol"));
			if (cursol) {
				cursol->isDraw = false;
				cursol->StopSpriteAnimation();
			}
		}

		void ReturnToTitleMenu::PlaySelectedAnimation()
		{
		}

		void ReturnToTitleMenu::InitializeLogic()
		{
			// サウンドバーの位置情報設定
			// アニメーションとかいれたり

			/** キャンバス（UI全体) */
			auto* canvas = GetCanvas();
			if (canvas)
			{
				canvas->transform.localScale = Vector3::Zero;
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

