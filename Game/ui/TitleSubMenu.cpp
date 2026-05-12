/**
 * TitleSubMenu.cpp
 */
#include "stdafx.h"
#include "TitleSubMenu.h"
#include "ui/UIAnimationFactory.h" 
#include "ui/UIAnimation.h"
#include "core/TitleMenuManager.h"

namespace app {
    namespace ui {
        TitleSubMenu::TitleSubMenu() {}
        TitleSubMenu::~TitleSubMenu() {}

        void TitleSubMenu::InitializeLogic() {
            highlight_ = GetUI<UIIcon>(Hash32("Highlight"));

            if (highlight_) {
                app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(highlight_, Hash32("FadeIn"));
                auto* anim = highlight_->FindAnimation(Hash32("FadeIn"));
                if (anim) anim->Play();
            }
        }

        void TitleSubMenu::Update() {
            MenuBase::Update();

            if (g_pad[0]->IsTrigger(enButtonUp)) {
                currentIndex_--;
                if (currentIndex_ < 0) currentIndex_ = maxIndex_;
            }
            else if (g_pad[0]->IsTrigger(enButtonDown)) {
                currentIndex_++;
                if (currentIndex_ > maxIndex_) currentIndex_ = 0;
            }

            if (highlight_) {
                if (currentIndex_ == 0) highlight_->transform.localPosition.y = -225.0f;
                else if (currentIndex_ == 1) highlight_->transform.localPosition.y = -305.0f;
                else if (currentIndex_ == 2) highlight_->transform.localPosition.y = -385.0f;
            }

            if (g_pad[0]->IsTrigger(enButtonA)) {
                switch (currentIndex_) {
                case 0: // GAME START
                    isGameStartDecided_ = true;
                    break;
                case 1: // SYSTEM
                    // TODO: 
                    OnClose();
                    app::core::TitleMenuManager::Get().ChangeState(app::core::TitleMenuState::enSoundMenu);
                    break;
                case 2: // EXIT
                    // TODO: 
                    PostQuitMessage(0);
                    break;
                }
            }
        }

        void TitleSubMenu::OnOpen() {
            if (highlight_) {
                highlight_->isDraw = true;
                auto* anim = highlight_->FindAnimation(Hash32("FadeIn"));
                if (anim) anim->Play();
            }
        }

        void TitleSubMenu::OnClose() {
            if (highlight_) {
                highlight_->isDraw = false;
                highlight_->StopSpriteAnimation();
            }
        }
    }
}