#include "stdafx.h"
#include "ui/GameOverSequence.h"
#include "ui/ResultSubMenu.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"
#include "sound/SoundManager.h"

namespace app {
    namespace ui {
        GameOverSequence::GameOverSequence() {
            layout_ = std::make_unique<app::ui::Layout>();
            layout_->Initialize<app::ui::MenuBase>("Assets/ui/layout/gameOverLayout.json");
        }

        GameOverSequence::~GameOverSequence() {}

        void GameOverSequence::StartSequence() {
            currentState_ = SequenceState::Wait;
            delayTimer_ = 1.0f;

            auto* menu = layout_->GetMenu();
            if (menu) {
                auto* fog = menu->GetUI<UIIcon>(Hash32("GameOverFog"));
                auto* word = menu->GetUI<UIIcon>(Hash32("GameOver"));
                if (fog) {
                    fog->isDraw = false;
                    fog->color.w = 0.0f;
                    fog->transform.localScale.x = 0.0f;
                }
                if (word) {
                    word->isDraw = false;
                    word->color.w = 0.0f;
                }
            }
        }

        void GameOverSequence::Update() {
            if (currentState_ == SequenceState::Standby) return;

            layout_->Update();

            auto* menu = layout_->GetMenu();
            if (!menu) return;

            auto* fog = menu->GetUI<UIIcon>(Hash32("GameOverFog"));
            auto* word = menu->GetUI<UIIcon>(Hash32("GameOver"));

            // ---- Wait ----
            if (currentState_ == SequenceState::Wait) {
                if (fog) fog->isDraw = false;
                if (word) word->isDraw = false;

                delayTimer_ -= g_gameTime->GetFrameDeltaTime();
                if (delayTimer_ <= 0.0f) {
                    currentState_ = SequenceState::FadeIn;
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::GameOverVoice));

                    if (fog) {
                        fog->color.w = 0.0f;
                        fog->transform.localScale.x = 0.0f;
                        app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(fog, Hash32("GameOverFadeIn"));
                        auto* animColor = fog->FindAnimation(Hash32("GameOverFadeIn"));
                        if (animColor) animColor->Play();
                        app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(fog, Hash32("GameOverFogExpand"));
                        auto* animScale = fog->FindAnimation(Hash32("GameOverFogExpand"));
                        if (animScale) animScale->Play();
                        fog->isDraw = true;
                    }
                    if (word) {
                        word->color.w = 0.0f;
                        app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(word, Hash32("GameOverFadeIn"));
                        auto* anim = word->FindAnimation(Hash32("GameOverFadeIn"));
                        if (anim) anim->Play();
                        word->isDraw = true;
                    }
                }
            }
            // ---- FadeIn ----
            else if (currentState_ == SequenceState::FadeIn) {
                if (fog) fog->isDraw = true;
                if (word) word->isDraw = true;

                bool isPlaying = false;
                if (word) {
                    auto* anim = word->FindAnimation(Hash32("GameOverFadeIn"));
                    if (anim && anim->IsPlay()) isPlaying = true;
                }

                if (!isPlaying) {
                    currentState_ = SequenceState::Finished;
                    menuWaitTimer_ = 1.5f;
                }
            }
            // ---- Finished（表示したまま一定時間待機） ----
            else if (currentState_ == SequenceState::Finished) {
                if (fog) fog->isDraw = true;
                if (word) word->isDraw = true;

                menuWaitTimer_ -= g_gameTime->GetFrameDeltaTime();
                if (menuWaitTimer_ <= 0.0f) {
                    currentState_ = SequenceState::FadeOut;
                    fadeOutTimer_ = 0.0f;
                }
            }
            // ---- FadeOut（GAMEOVERをフェードアウト） ----
            else if (currentState_ == SequenceState::FadeOut) {
                fadeOutTimer_ += g_gameTime->GetFrameDeltaTime();
                float t = fadeOutTimer_ / kFadeOutDuration;
                if (t > 1.0f) t = 1.0f;

                float alpha = 1.0f - t;
                if (fog) {
                    fog->isDraw = true;
                    fog->color.w = alpha;
                }
                if (word) {
                    word->isDraw = true;
                    word->color.w = alpha;
                }

                if (t >= 1.0f) {
                    if (fog) fog->isDraw = false;
                    if (word) word->isDraw = false;

                    currentState_ = SequenceState::ShowMenu;
                    subMenuLayout_ = std::make_unique<app::ui::Layout>();
                    subMenuLayout_->Initialize<app::ui::ResultSubMenu>("Assets/ui/layout/ResultMenuLayout.json");
                    auto* subMenu = dynamic_cast<app::ui::ResultSubMenu*>(subMenuLayout_->GetMenu());
                    if (subMenu) subMenu->OnOpen();
                }
            }
            // ---- ShowMenu ----
            else if (currentState_ == SequenceState::ShowMenu) {
                if (fog) fog->isDraw = false;
                if (word) word->isDraw = false;
                if (subMenuLayout_) subMenuLayout_->Update();
            }
        }

        void GameOverSequence::Render(RenderContext& rc) {
            if (currentState_ == SequenceState::Standby) return;
            if (layout_) layout_->Render(rc);
            if (currentState_ == SequenceState::ShowMenu && subMenuLayout_) {
                subMenuLayout_->Render(rc);
            }
        }

        bool GameOverSequence::IsReturnTitleDecided() const {
            if (subMenuLayout_) {
                auto* subMenu = dynamic_cast<app::ui::ResultSubMenu*>(subMenuLayout_->GetMenu());
                if (subMenu) return subMenu->IsReturnTitleDecided();
            }
            return false;
        }

        bool GameOverSequence::IsRetryDecided() const {
            if (subMenuLayout_) {
                auto* subMenu = dynamic_cast<app::ui::ResultSubMenu*>(subMenuLayout_->GetMenu());
                if (subMenu) return subMenu->IsRetryDecided();
            }
            return false;
        }

        bool GameOverSequence::IsExitDecided() const {
            if (subMenuLayout_) {
                auto* subMenu = dynamic_cast<app::ui::ResultSubMenu*>(subMenuLayout_->GetMenu());
                if (subMenu) return subMenu->IsExitDecided();
            }
            return false;
        }
    }
}
