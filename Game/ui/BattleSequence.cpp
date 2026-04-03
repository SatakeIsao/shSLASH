#include "stdafx.h"
#include "BattleSequence.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"

namespace
{
	static app::ui::UIAnimationSequence* seq = nullptr;
}

namespace app
{
	namespace ui
	{
		BattleSequence::BattleSequence()
		{
			layout_ = std::make_unique<app::ui::Layout>();
			layout_->Initialize<app::ui::MenuBase>("Assets/ui/layout/BattleSequenceMenuLayout.json");
			currentDown_ = SequenceName::Wait;

			auto* menu = layout_->GetMenu();
			if (menu)
			{
				auto* readyIcon = menu->GetUI<UIIcon>(Hash32("Ready"));
				if (readyIcon)
				{
					app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(readyIcon, Hash32("ScaleUp_Ready"));
				}
			}
		}

		BattleSequence::~BattleSequence()
		{
		}

		void BattleSequence::Update()
		{
			// 【重要】先にレイアウトを更新（ここでホットリロードによるUI再生成が行われるため）
			layout_->Update();
			auto* menu = layout_->GetMenu();

			// タイマーの更新
			if (currentDown_ == SequenceName::Wait)
			{
				delayTimer_ -= g_gameTime->GetFrameDeltaTime();
				if (delayTimer_ <= 0.0f)
				{
					currentDown_ = SequenceName::Ready;

					if (menu)
					{
						auto* readyIcon = menu->GetUI<UIIcon>(Hash32("Ready"));
						if (readyIcon)
						{
							//seq = new app::ui::UIAnimationSequence();
							//seq->Add(Hash32("ScaleUp_Ready"));
							//seq->Play(readyIcon);

							/** DEBUG_TEST: UIAnimシーケンスで再生したい */
							app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(readyIcon, Hash32("ScaleUp_Ready"));
							auto* anim = readyIcon->FindAnimation(Hash32("ScaleUp_Ready"));
							if (anim) anim->Play();
						}
					}
				}
			}
			else if (currentDown_ == SequenceName::Ready)
			{
				maxCountDownTimer_ -= g_gameTime->GetFrameDeltaTime();
				if (maxCountDownTimer_ <= 0.0f)
				{
					currentDown_ = SequenceName::GO;
					if (menu)
					{
						auto* goIcon = menu->GetUI<UIIcon>(Hash32("Go"));
						if (goIcon)
						{
							app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(goIcon, Hash32("ScaleUp_Go"));
							auto* anim = goIcon->FindAnimation(Hash32("ScaleUp_Go"));
							if (anim) anim->Play();
						}
					}

				}
			}
			else if (currentDown_ == SequenceName::GO)
			{
				goTimer_ -= g_gameTime->GetFrameDeltaTime();
				if (goTimer_ <= 0.0f)
				{
					currentDown_ = SequenceName::Finished;
				}
			}

			// 毎フレーム安全にUIを取得し、表示状態を上書きする
			//auto* menu = layout_->GetMenu();
			if (menu)
			{
				// 事前計算したハッシュ値を使うので超高速
				auto* readyIcon = menu->GetUI<UIIcon>(Hash32("Ready"));
				auto* goIcon = menu->GetUI<UIIcon>(Hash32("Go"));

				// isDraw フラグを使って物理的に描画をON/OFFする
				if (currentDown_ == SequenceName::Wait || currentDown_ == SequenceName::Finished)
				{
					if (readyIcon) readyIcon->isDraw = false;
					if (goIcon) goIcon->isDraw = false;
				}
				else if (currentDown_ == SequenceName::Ready)
				{
					if (readyIcon) readyIcon->isDraw = true;
					if (goIcon)    goIcon->isDraw = false;
				}
				else if (currentDown_ == SequenceName::GO)
				{
					if (readyIcon) readyIcon->isDraw = false;
					if (goIcon)    goIcon->isDraw = true;
				}
			}
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
