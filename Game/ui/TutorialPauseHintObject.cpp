#include "stdafx.h"
#include "TutorialPauseHintObject.h"
#include "core/PauseManager.h"

namespace app
{
    namespace ui
    {
        bool TutorialPauseHintObject::Start()
        {
            layout_ = std::make_unique<Layout>();
            layout_->Initialize<MenuBase>("Assets/ui/layout/tutorialPauseHintLayout.json");
            return true;
        }

        void TutorialPauseHintObject::Update()
        {
            if (layout_) layout_->Update();
        }

        void TutorialPauseHintObject::Render(RenderContext& rc)
        {
            if (!app::core::PauseManager::IsAvailable()) return;
            if (!app::core::PauseManager::Get().IsInPauseMenu()) return;
            if (layout_) layout_->Render(rc);
        }
    }
}
