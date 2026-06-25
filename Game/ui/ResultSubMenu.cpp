#include "stdafx.h"
#include "ResultSubMenu.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"
#include "sound/SoundManager.h"

namespace app {
    namespace ui {
        ResultSubMenu::ResultSubMenu() {}
        ResultSubMenu::~ResultSubMenu() {}

        void ResultSubMenu::InitializeLogic() {
            // UIパーツの取得
            backFilter_ = GetUI<UIIcon>(Hash32("BackFilter"));
            menuLine1_ = GetUI<UIIcon>(Hash32("MenuLine"));
            menuLine2_ = GetUI<UIIcon>(Hash32("MenuLine_2"));
            menuBase_ = GetUI<UIIcon>(Hash32("ResultMenuBase"));
            retry_ = GetUI<UIIcon>(Hash32("Retry"));
            title_ = GetUI<UIIcon>(Hash32("TITLE"));
            exit_ = GetUI<UIIcon>(Hash32("Exit"));
            highlight_ = GetUI<UIIcon>(Hash32("Highlight"));

            cursorIndex_ = 0;
            waitFrame_ = 0;
            fadeProgress_ = 0.0f;

            UIIcon* allTargets[] = { retry_, title_, exit_, backFilter_, menuBase_, menuLine1_, menuLine2_, highlight_ };
            for (auto* ui : allTargets) {
                if (ui) {
                    ui->isDraw = false;
                    ui->color.w = 0.0f;
                }
            }

            if (highlight_) {
                app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(highlight_, Hash32("FadeIn"));

                float baseX = highlight_->transform.localPosition.x;
                float baseZ = highlight_->transform.localPosition.z;

                // Vector3(X座標, Y座標, Z座標)
                cursorPositions_[0] = Vector3(baseX, 93.0f, baseZ); // Retry のカーソル位置
                cursorPositions_[1] = Vector3(baseX, -13.0f, baseZ); // Title のカーソル位置
                cursorPositions_[2] = Vector3(baseX, -119.0f, baseZ); // Exit  のカーソル位置

                highlight_->transform.localPosition = cursorPositions_[0];
            }
        }

        void ResultSubMenu::OnOpen() {
            waitFrame_ = 5;
            fadeProgress_ = 0.0f;
        }

        void ResultSubMenu::StartFadeIn() {
        }

        void ResultSubMenu::Update() {
            if (waitFrame_ > 0) {
                waitFrame_--;
                return; 
            }
            
            //フェードイン処理
            if (fadeProgress_ < 1.0f) {
                fadeProgress_ += g_gameTime->GetFrameDeltaTime() * 4.0f;
                if (fadeProgress_ > 1.0f) fadeProgress_ = 1.0f;

                UIIcon* opaqueTargets[] = { retry_, title_, exit_ };
                for (auto* ui : opaqueTargets) {
                    if (ui) {
                        ui->isDraw = true;
                        ui->color.w = fadeProgress_;
                    }
                }

                UIIcon* halfTargets[] = { backFilter_, menuBase_, menuLine1_, menuLine2_ };
                for (auto* ui : halfTargets) {
                    if (ui) {
                        ui->isDraw = true;
                        ui->color.w = fadeProgress_ * 0.6f;
                    }
                }

                // 点滅を開始
                if (fadeProgress_ == 1.0f) {
                    if (highlight_) {
                        highlight_->isDraw = true;
                        highlight_->color.w = 1.0f;
                        auto* anim = highlight_->FindAnimation(Hash32("FadeIn"));
                        if (anim) anim->Play();
                    }
                }
            }

            MenuBase::Update();

            if (fadeProgress_ >= 1.0f) {
                // カーソル移動処理
                if (g_pad[0]->IsTrigger(enButtonDown)) {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonMove));
                    cursorIndex_++;
                    if (cursorIndex_ > 2) cursorIndex_ = 0;
                }
                if (g_pad[0]->IsTrigger(enButtonUp)) {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonMove));
                    cursorIndex_--;
                    if (cursorIndex_ < 0) cursorIndex_ = 2;
                }

                // Highlight の座標更新
                if (highlight_) {
                    highlight_->transform.localPosition = cursorPositions_[cursorIndex_];
                    highlight_->transform.UpdateTransform();
                }

                // 決定処理
                if (g_pad[0]->IsTrigger(enButtonA)) {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonDecision));
                    if (cursorIndex_ == 0)      isRetryDecided_ = true;
                    else if (cursorIndex_ == 1) isReturnTitleDecided_ = true;
                    else if (cursorIndex_ == 2) isExitDecided_ = true;
                }
            }
        }
    }
}