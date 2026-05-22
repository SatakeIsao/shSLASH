#include "stdafx.h"
#include "EnemyPhase.h"

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
	fontRender_.SetText(L"Phase 1");
}   

void app::actor::PhaseUI::Update()
{
	fontRender_.SetPosition(-100.0f, 500.0f, 0.0f);
	fontRender_.SetScale(1.0f);
    fontRender_.SetColor(Vector4::White);
}

void app::actor::PhaseUI::Render(RenderContext& rc)
{
	fontRender_.Draw(rc);
}
