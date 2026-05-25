#include "stdafx.h"
#include "ui/GameOverSequence.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"

namespace app {
    namespace ui {
        GameOverSequence::GameOverSequence() {
            layout_ = std::make_unique<app::ui::Layout>();
            layout_->Initialize<app::ui::MenuBase>("Assets/ui/layout/gameOverLayout.json");
        }

        GameOverSequence::~GameOverSequence() {}

        void GameOverSequence::StartSequence() {
            currentState_ = SequenceState::Wait;
            delayTimer_ = 1.0f; // 倒れてから1秒間は何も出さない

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

            if (currentState_ == SequenceState::Wait) {
                if (fog) fog->isDraw = false;
                if (word) word->isDraw = false;

                delayTimer_ -= g_gameTime->GetFrameDeltaTime();
                if (delayTimer_ <= 0.0f) {
                    currentState_ = SequenceState::FadeIn;

                    // アニメーションをアタッチして再生
                    if (fog) {
                        fog->color.w = 0.0f;
                        fog->transform.localScale.x = 0.0f;

                        // フェードイン
                        app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(fog, Hash32("GameOverFadeIn"));
                        auto* animColor = fog->FindAnimation(Hash32("GameOverFadeIn"));
                        if (animColor) animColor->Play();

                        // 横に広がるスケール
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
            // フェードイン
            else if (currentState_ == SequenceState::FadeIn) {
                // 表示状態を維持
                if (fog) fog->isDraw = true;
                if (word) word->isDraw = true;

                // 文字のアニメーションが終わったか監視する
                bool isPlaying = false;
                if (word) {
                    auto* anim = word->FindAnimation(Hash32("GameOverFadeIn"));
                    if (anim && anim->IsPlay()) {
                        isPlaying = true;
                    }
                }

                if (!isPlaying) {
                    currentState_ = SequenceState::Finished; // 再生が終わったら次へ
                }
            }
            // 完了状態
            else if (currentState_ == SequenceState::Finished) {
                if (fog) fog->isDraw = true;
                if (word) word->isDraw = true;

                // アニメーションが完全に出現してから、Aボタンでタイトルへ戻る
                if (g_pad[0]->IsTrigger(enButtonA)) {
                    isReturnTitleDecided_ = true;
                }
            }
        }

        void GameOverSequence::Render(RenderContext& rc) {
            if (currentState_ != SequenceState::Standby && layout_) {
                layout_->Render(rc);
            }
        }
    }
}