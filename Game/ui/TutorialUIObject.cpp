#include "stdafx.h"
#include "TutorialUIObject.h"
#include "battle/BattleManager.h"

namespace app
{
    namespace ui
    {
        TutorialUIObject::TutorialUIObject()
        {
            layout_ = std::make_unique<Layout>();
            layout_->Initialize<TutorialMessageMenu>("Assets/ui/layout/tutorialUILayout.json");
        }

        void TutorialUIObject::Update()
        {
            if (!isVisible_)
            {
                if (app::battle::BattleManager::IsAvailable() &&
                    app::battle::BattleManager::Get().IsOpeningSequenceDone())
                {
                    isVisible_ = true;
                }
                return;
            }
            if (layout_) layout_->Update();
        }

        void TutorialUIObject::Render(RenderContext& rc)
        {
            if (!isVisible_) return;
            if (layout_) layout_->Render(rc);
        }

        bool TutorialUIObject::IsAllMessagesShown() const
        {
            if (!layout_) return false;
            auto* menu = dynamic_cast<TutorialMessageMenu*>(layout_->GetMenu());
            if (!menu) return false;
            return menu->IsAllMessagesShown();
        }

        bool TutorialUIObject::IsPlayerInputAllowed() const
        {
            if (!layout_) return false;
            auto* menu = dynamic_cast<TutorialMessageMenu*>(layout_->GetMenu());
            if (!menu) return false;
            return menu->IsPlayerInputAllowed();
        }

        bool TutorialUIObject::IsPracticePhaseReached() const
        {
            if (!layout_) return false;
            auto* menu = dynamic_cast<TutorialMessageMenu*>(layout_->GetMenu());
            if (!menu) return false;
            return menu->IsPracticePhaseReached();
        }

        void TutorialUIObject::SetTimerProgress(float elapsed)
        {
            if (!layout_) return;
            auto* menu = dynamic_cast<TutorialMessageMenu*>(layout_->GetMenu());
            if (!menu) return;
            menu->SetTimerProgress(elapsed);
        }

        void TutorialUIObject::TriggerFinalClear()
        {
            if (!layout_) return;
            auto* menu = dynamic_cast<TutorialMessageMenu*>(layout_->GetMenu());
            if (menu) menu->TriggerFinalClear();
        }

        bool TutorialUIObject::IsFinalClearDone() const
        {
            if (!layout_) return false;
            auto* menu = dynamic_cast<TutorialMessageMenu*>(layout_->GetMenu());
            return menu ? menu->IsFinalClearDone() : false;
        }
    }
}
