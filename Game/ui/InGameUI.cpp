#include "stdafx.h"
#include "InGameUI.h"

namespace {
	static const int MAX_TIME = 10;
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
			layout_ = std::make_unique<app::ui::Layout>();
			layout_->Initialize <app::ui::MenuBase>("Assets/ui/layout/hpLayout.json");
		}

		HpUIObject::~HpUIObject()
		{}

		void HpUIObject::Update()
		{
			if (!layout_) return; // layout自体がなければ何もしない

			auto menu = layout_->GetMenu();
			if (menu)
			{
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
