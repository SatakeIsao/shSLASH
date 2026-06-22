#include "stdafx.h"
#include "TutorialReturnConfirmMenu.h"
#include "sound/SoundManager.h"

namespace app
{
    namespace ui
    {
        void TutorialReturnConfirmMenu::InitializeLogic()
        {
            yesIcon_ = GetUI<UIIcon>(Hash32("Text_Yes"));
            noIcon_  = GetUI<UIIcon>(Hash32("Text_No"));
            cursor_  = GetUI<UIIcon>(Hash32("Cursor"));
            if (yesIcon_) yesIcon_->color = Vector4::White;
            if (noIcon_)  noIcon_->color  = Vector4::White;
            ApplyCursor();
        }

        void TutorialReturnConfirmMenu::Update()
        {
            MenuBase::Update();

            if (g_pad[0]->IsTrigger(enButtonUp))
            {
                if (currentIndex_ != 0)
                {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonMove));
                    currentIndex_ = 0;
                    ApplyCursor();
                }
            }
            else if (g_pad[0]->IsTrigger(enButtonDown))
            {
                if (currentIndex_ != 1)
                {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonMove));
                    currentIndex_ = 1;
                    ApplyCursor();
                }
            }

            if (g_pad[0]->IsTrigger(enButtonA))
            {
                app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonDecision));
                if (currentIndex_ == 0)
                    isYesDecided_ = true;
                else
                    isNoDecided_ = true;
            }
            else if (g_pad[0]->IsTrigger(enButtonB))
            {
                app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonReturn));
                isNoDecided_ = true;
            }
        }

        bool TutorialReturnConfirmMenu::IsYesDecided() const { return isYesDecided_; }
        bool TutorialReturnConfirmMenu::IsNoDecided()  const { return isNoDecided_;  }

        void TutorialReturnConfirmMenu::ResetState()
        {
            isYesDecided_ = false;
            isNoDecided_  = false;
            currentIndex_ = 1; // デフォルト NO
            ApplyCursor();
        }

        void TutorialReturnConfirmMenu::ApplyCursor()
        {
            if (!cursor_) return;
            const float yesY =  55.0f;
            const float noY  = -55.0f;
            cursor_->transform.localPosition.y = (currentIndex_ == 0) ? yesY : noY;
            cursor_->transform.UpdateTransform();
        }
    }
}
