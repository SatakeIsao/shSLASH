#include "stdafx.h"
#include "ui/BattleGuideUIObject.h"

app::ui::BattleGuideUIObject::BattleGuideUIObject()
{
	layout_ = std::make_unique<app::ui::Layout>();
	layout_->Initialize<app::ui::MenuBase>("Assets/ui/layout/battleGuideLayout.json");
}

void app::ui::BattleGuideUIObject::Update()
{
	if (layout_)
	{
		layout_->Update();
	}
}

void app::ui::BattleGuideUIObject::Render(RenderContext& rc)
{
	if (layout_)
	{
		layout_->Render(rc);
	}
}
