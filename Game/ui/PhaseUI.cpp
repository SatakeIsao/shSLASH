#include "stdafx.h"
#include "ui/PhaseUI.h"
#include "ui/Menu.h"
#include "ui/UIParts.h"

app::ui::PhaseUI::PhaseUI()
{
    layout_ = std::make_unique<app::ui::Layout>();
    layout_->Initialize<app::ui::MenuBase>("Assets/ui/layout/phaseLayout.json");

    SetPhaseCount(1);
}

void app::ui::PhaseUI::SetPhaseCount(int phaseCount)
{
    if (currentPhase_ != phaseCount)
    {
        currentPhase_ = phaseCount;
    }
}

void app::ui::PhaseUI::Update()
{
    if (layout_ && layout_->GetMenu())
    {
        auto* menu = layout_->GetMenu();
        auto* numSprite = menu->GetUI<app::ui::UINumberSprite>(Hash32("PhaseNumber"));
        if (numSprite)
        {
            numSprite->SetNumber(currentPhase_);
        }
        layout_->Update();
    }
}

void app::ui::PhaseUI::Render(RenderContext& rc)
{
    if (layout_)
    {
        layout_->Render(rc);
    }
}
