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
					//TODO; X座標もやりたいなぁ
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

			// 動的に数値をUIに設定
			auto* parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterPauseMenuParameter>();
			// 音を変える/タイトルにもどるテキストの場所
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

		void PauseMenu::OnOpen()
		{
			isPause_ = true;

			//キャンバス
			{
				auto* canvas = GetCanvas();
				if (canvas)
				{
					canvas->transform.localScale = Vector3::Zero;
					canvas->RemoveAnimation(Hash32("ScaleUp"));
					canvas->RemoveAnimation(Hash32("ScaleDown"));
					//アニメーションをアタッチ
					app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(canvas, Hash32("ScaleUp"));
					auto* openAnim = canvas->FindAnimation(Hash32("ScaleUp"));
					if (openAnim) openAnim->Play();
				}
			}
		}

		void PauseMenu::OnClose()
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
		}

		void PauseMenu::PlaySelectedAnimation()
		{
			/** TODO: Updateで、処理が走っているので、無駄な処理を改善したい */
			auto* textSound = GetUI<app::ui::UIIcon>(Hash32("text_ChangeTheSound"));
			auto* textTitle = GetUI<app::ui::UIIcon>(Hash32("text_ReturnToTitle"));

			/** 各項目選択中に拡大アニメーションを再生 */
			if (cursolIndex_ == 0
				&& textSound)
			{
				/** リセット: 黄色から白 */
				textTitle->color.Set(Vector3(1.0f, 1.0f, 1.0f));
				/** リセット: 等倍に戻す */
				textTitle->transform.localScale = Vector3(1.0f, 1.0f, 0.0f);

				/** 黄色 */
				textSound->color.Set(Vector3(1.0f, 1.0f, 0.0f));
				/** スケール拡大 */
				textSound->transform.localScale = Vector3(1.3f, 1.3f, 0.0f);
			}
			else if (cursolIndex_ == 1
				&& textTitle)
			{
				/** リセット: 黄色から白 */
				textSound->color.Set(Vector3(1.0f, 1.0f, 1.0f));
				/** リセット: 等倍に戻す */
				textSound->transform.localScale = Vector3(1.0f, 1.0f, 0.0f);

				/** 黄色 */
				textTitle->color.Set(Vector3(1.0f, 1.0f, 0.0f));
				/** スケール拡大 */
				textTitle->transform.localScale = Vector3(1.3f, 1.3f, 0.0f);
			}
		}

		void PauseMenu::InitializeLogic()
		{
			// サウンドバーの位置情報設定
			// アニメーションとかいれたり
			/** キャンバス（UI全体) */
			{
				auto* canvas = GetCanvas();
				if (canvas)
				{
					canvas->transform.localScale = Vector3::Zero;
				}
			}
		}
	}
}