#include "stdafx.h"
#include "EnemyPhase.h"
#include "ui/Menu.h"
#include "ui/UIParts.h"

bool app::actor::EnemyPhase::Update(int playerLevel, EnemyPhaseData& outData)
{
    for(int i = static_cast<int>(phases_.size()) - 1; i >= 0; --i)
    {
        if(playerLevel >= phases_[i].requiredPlayerLevel)
        {
            if (currentIndex_ == i) return false;

			currentIndex_ = i;
			outData = phases_[i];
            phaseCount_++;
            return true;
        }
	}
    return false;
}

app::actor::PhaseUI::PhaseUI()
{
    // InGameUIクラス等の実装に倣い、MenuBase型としてLayoutを初期化
    layout_ = std::make_unique<app::ui::Layout>();
    layout_->Initialize<app::ui::MenuBase>("Assets/ui/layout/phaseLayout.json");

    // 初期状態としてフェーズ1を設定
    SetPhaseCount(1);
}   

void app::actor::PhaseUI::SetPhaseCount(int phaseCount)
{
    if (currentPhase_ != phaseCount)
    {
        currentPhase_ = phaseCount;
        // （フェーズが変わった時の処理があればここに書く）
    }
}

void app::actor::PhaseUI::Update()
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

void app::actor::PhaseUI::Render(RenderContext& rc)
{
    if (layout_)
    {
        layout_->Render(rc);
    }
}
