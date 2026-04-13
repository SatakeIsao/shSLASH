#include "stdafx.h"
#include "InGameUI.h"
#include "core/ParameterManager.h"

namespace {
	static const int MAX_TIME = 10;
	static const int MAX_LEVEL = 10;
}

namespace app
{
	namespace ui
	{
		TimerUIObject::TimerUIObject()
		{
			layout_ = std::make_unique<app::ui::Layout>();
			layout_->Initialize <app::ui::MenuBase>("Assets/ui/layout/timerLayout.json");

			timer_ = MAX_TIME;
		}

		TimerUIObject::~TimerUIObject()
		{
		}

		void TimerUIObject::Update()
		{
			if (timer_ <= 0.0f) {
				timer_ = 0.0f;
			}

			if (!layout_) return; // layout自体がなければ何もしない

			auto menu = layout_->GetMenu();
			if (menu) 
			{
				// 取得に失敗（nullptr）した場合の安全策を強化
				auto timerDigit = menu->GetUI<app::ui::UIDigit>(Hash32("timerNumbers"));
				if (timerDigit)
				{
					timerDigit->SetZeroPadding(true);
					timerDigit->SetNumber(static_cast<int>(std::ceil(timer_)));
				}
			}
			layout_->Update();
		}

		void TimerUIObject::Render(RenderContext& rc)
		{
			if (layout_) {
				layout_->Render(rc);
			}
		}




		/**********************************************/


		HpUIObject::HpUIObject()
		{
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterHpUIParameter>("Assets/master/HpUIParameter.json", [](const nlohmann::json& j, app::core::MasterHpUIParameter& p)
				{
					// hpバー座標X
					char hpBarPositionX[] = "hpBarPositionXA";
					const uint32_t barPosX = ARRAYSIZE(p.hpBarPositionX);
					for (uint32_t i = 0; i < barPosX; ++i) {
						hpBarPositionX[14] = 'A' + i;
						p.hpBarPositionX[i] = j[hpBarPositionX];
					}
					
					// HPバーのスケールX
					char hpBarScaleX[] = "hpBarScaleXA";
					const uint32_t barScaleX = ARRAYSIZE(p.hpBarScaleX);
					for (uint32_t i = 0; i < barScaleX; ++i) {
						hpBarScaleX[11] = 'A' + i;
						p.hpBarScaleX[i] = j[hpBarScaleX];
					}

					// レベルバー座標X
					char levelBarPositionX[] = "levelBarPositionXA";
					const uint32_t PosX = ARRAYSIZE(p.levelBarPositionX);
					for (uint32_t i = 0; i < PosX; ++i) {
						levelBarPositionX[17] = 'A' + i;
						p.levelBarPositionX[i] = j[levelBarPositionX];
					}

					// レベルバーのスケールX
					char levelBarScaleX[] = "levelBarScaleXA";
					const uint32_t ScaleX = ARRAYSIZE(p.levelBarScaleX);
					for (uint32_t i = 0; i < ScaleX; ++i) {
						levelBarScaleX[14] = 'A' + i;
						p.levelBarScaleX[i] = j[levelBarScaleX];
					}
				});

			auto* parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterHpUIParameter>();

			if (parameter)
			{
				damagePosX_ = parameter->hpBarPositionX[0];
				damageScaleX_ = parameter->hpBarScaleX[0];
			}

			layout_ = std::make_unique<app::ui::Layout>();
			layout_->Initialize <app::ui::MenuBase>("Assets/ui/layout/hpLayout.json");

			auto currentLevel = layout_->GetMenu()->GetUI<UIIcon>(Hash32("currentLevel"));
			if (currentLevel)
			{
				currentLevel->transform.localPosition.x = parameter->levelBarPositionX[0];
				currentLevel->transform.localScale.x = parameter->levelBarScaleX[0];
			}
		}

		HpUIObject::~HpUIObject()
		{}

		void HpUIObject::Update()
		{
			if (!layout_) return;

			auto* parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterHpUIParameter>();

			/** HPバー座標X */
			{
				auto currentHP = layout_->GetMenu()->GetUI<UIIcon>(Hash32("currentHP"));
				auto damageHP = layout_->GetMenu()->GetUI<UIIcon>(Hash32("damageHP"));
				auto currentLevel = layout_->GetMenu()->GetUI<UIIcon>(Hash32("currentLevel"));

				if (!currentHP || !damageHP || !currentLevel) return;


				if (level_ >= MAX_LEVEL)
				{
					levelUpIndex_ = MAX_LEVEL;
					if (currentLevel)
					{
						currentLevel->transform.localPosition.x = parameter->levelBarPositionX[MAX_LEVEL];
						currentLevel->transform.localScale.x = parameter->levelBarScaleX[MAX_LEVEL];
					}
				}

				// 即時反映
				if (g_pad[0]->IsTrigger(enButtonLeft))
				{
					index_ = max(0, index_ - 1);
					// タイマーリセット
					damageDelayTimer_ = kDamageDelayTime;
					lerpVal_ = 0.0f;
					// 現在地を保存
					damagePosX_ = damageHP->transform.localPosition.x;
					damageScaleX_ = damageHP->transform.localScale.x;

					currentHP->transform.localPosition.x = parameter->hpBarPositionX[index_];
					currentHP->transform.localScale.x = parameter->hpBarScaleX[index_];
				}
				/** デバッグテスト： 右ボタン */
				if (g_pad[0]->IsTrigger(enButtonRight))
				{
					index_ = min(MAX_LEVEL, index_ + 1);

					// ★追加: Lv.10ならlevelUpIndex_を変動させない
					if (level_ >= MAX_LEVEL)
					{
						levelUpIndex_ = MAX_LEVEL;
					}
					else
					{
						/** 折り返し時にレベルアップ */
						if (levelUpIndex_ >= MAX_LEVEL)
						{
							levelUpIndex_ = 0;
							level_ = min(MAX_LEVEL, level_ + 1);
						}
						else
						{
							levelUpIndex_ = levelUpIndex_ + 1;
						}
					}
					
					// タイマーリセット
					damageDelayTimer_ = kDamageDelayTime;
					lerpVal_ = 0.0f;
					// 現在地を保存
					damagePosX_ = damageHP->transform.localPosition.x;
					damageScaleX_ = damageHP->transform.localScale.x;
					 
					currentHP->transform.localPosition.x = parameter->hpBarPositionX[index_];
					currentHP->transform.localScale.x = parameter->hpBarScaleX[index_];

					/** 今のレベルバー */
					currentLevel->transform.localPosition.x = parameter->levelBarPositionX[levelUpIndex_];
					currentLevel->transform.localScale.x = parameter->levelBarScaleX[levelUpIndex_];
				}

				// ディレイとLerp更新
				if (lerpVal_ < 1.0f && damageDelayTimer_ < 0.0f)
				{
					lerpVal_ += 1.0f * g_gameTime->GetFrameDeltaTime();
					lerpVal_ = min(lerpVal_, 1.0f);

					const float targetPosX = parameter->hpBarPositionX[index_];
					const float targetScaleX = parameter->hpBarScaleX[index_];

					// 開始地点と目標地点を補間
					float currentPosX = (targetPosX * lerpVal_) + (damagePosX_ * (1.0f - lerpVal_));
					float currentScaleX = (targetScaleX * lerpVal_) + (damageScaleX_ * (1.0f - lerpVal_));

					damageHP->transform.localPosition.x = currentPosX;
					damageHP->transform.localScale.x = currentScaleX;
				}
				else if (damageDelayTimer_ >= 0.0f)
				{
					damageDelayTimer_ -= g_gameTime->GetFrameDeltaTime();
				}
			}

			/** レベル数値の表示 */
			{
				auto levelDigit = layout_->GetMenu()->GetUI<app::ui::UIDigit>(Hash32("levelNumbers"));
				if (levelDigit)
				{
					//levelDigit->SetZeroPadding(true);
					levelDigit->SetNumber(level_);
				}
			}
			layout_->Update();
		}

		void HpUIObject::Render(RenderContext& rc)
		{
			if (layout_) {
				layout_->Render(rc);
			}
		}
	}
}
