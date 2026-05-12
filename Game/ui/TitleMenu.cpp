#include "stdafx.h"
#include "TitleMenu.h"
#include "ui/UIAnimationFactory.h" 
#include "ui/UIAnimation.h"        

namespace app {
    namespace ui {

        void TitleMenu::InitializeLogic() {
            pressA_ = GetUI<app::ui::UIIcon>(Hash32("GuideA"));
            titleLeft_ = GetUI<app::ui::UIIcon>(Hash32("TitleLeft"));
            titleRight_ = GetUI<app::ui::UIIcon>(Hash32("TitleRight"));

            if (pressA_) {
                app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(pressA_, Hash32("FadeIn"));
                auto* anim = pressA_->FindAnimation(Hash32("FadeIn"));
                if (anim) anim->Play();
            }
        }

        void TitleMenu::Update() {
            float dt = g_gameTime->GetFrameDeltaTime();
            if (dt <= 0.0f) dt = 1.0f / 60.0f;

            // --- 開く処理 ---
            if (animState_ == AnimState::Opening_Step1) {
                animTimer_ -= dt;
                if (animTimer_ <= 0.0f) {
                    animTimer_ = 0.4f;
                    if (titleLeft_) {
                        titleLeft_->RemoveAnimation(Hash32("TitleLogoSplitLeft_1"));
                        app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleLeft_, Hash32("TitleLogoSplitLeft_2"));
                        auto* anim = titleLeft_->FindAnimation(Hash32("TitleLogoSplitLeft_2"));
                        if (anim) anim->Play();
                    }
                    if (titleRight_) {
                        titleRight_->RemoveAnimation(Hash32("TitleLogoSplitRight_1"));
                        app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleRight_, Hash32("TitleLogoSplitRight_2"));
                        auto* anim = titleRight_->FindAnimation(Hash32("TitleLogoSplitRight_2"));
                        if (anim) anim->Play();
                    }
                }
            }

            else if (animState_ == AnimState::Closing) {
                animTimer_ -= dt;
                if (animTimer_ <= 0.0f) {
                    animState_ = AnimState::None;
                    if (pressA_) pressA_->isDraw = true;
                }
            }

            MenuBase::Update();
        }

        void TitleMenu::OnOpen() {
            animState_ = AnimState::Closing;
            animTimer_ = 0.4f;

            if (pressA_) pressA_->isDraw = false;

            if (titleLeft_) {
                titleLeft_->RemoveAnimation(Hash32("TitleLogoSplitLeft_2"));
                titleLeft_->RemoveAnimation(Hash32("TitleLogoReturnLeft"));
                app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleLeft_, Hash32("TitleLogoReturnLeft"));
                auto* anim = titleLeft_->FindAnimation(Hash32("TitleLogoReturnLeft"));
                if (anim) anim->Play();
            }

            if (titleRight_) {
                titleRight_->RemoveAnimation(Hash32("TitleLogoSplitRight_2"));
                titleRight_->RemoveAnimation(Hash32("TitleLogoReturnRight"));
                app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleRight_, Hash32("TitleLogoReturnRight"));
                auto* anim = titleRight_->FindAnimation(Hash32("TitleLogoReturnRight"));
                if (anim) anim->Play();
            }
        }

        void TitleMenu::OnClose() {
            animState_ = AnimState::Opening_Step1;
            animTimer_ = 0.8f;

            if (pressA_) pressA_->isDraw = false;

            if (titleLeft_) {
                titleLeft_->RemoveAnimation(Hash32("TitleLogoReturnLeft"));
                titleLeft_->RemoveAnimation(Hash32("TitleLogoSplitLeft_1"));
                app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleLeft_, Hash32("TitleLogoSplitLeft_1"));
                auto* anim = titleLeft_->FindAnimation(Hash32("TitleLogoSplitLeft_1"));
                if (anim) anim->Play();
            }

            if (titleRight_) {
                titleRight_->RemoveAnimation(Hash32("TitleLogoReturnRight"));
                titleRight_->RemoveAnimation(Hash32("TitleLogoSplitRight_1"));
                app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleRight_, Hash32("TitleLogoSplitRight_1"));
                auto* anim = titleRight_->FindAnimation(Hash32("TitleLogoSplitRight_1"));
                if (anim) anim->Play();
            }
        }

        bool TitleMenu::IsPlaying() {
            return animState_ != AnimState::None;
        }
    }
}