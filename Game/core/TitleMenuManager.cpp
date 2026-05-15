#include "stdafx.h"
#include "TitleMenuManager.h"
#include "ui/Layout.h"
#include "ui/Menu.h"
#include "ui/TitleMenu.h"   
#include "ui/TitleSubMenu.h"
#include "sound/SoundManager.h"
#include "ui/SoundOptionMenu.h"

namespace app {
    namespace core {
        TitleMenuManager* TitleMenuManager::instance_ = nullptr;

        TitleMenuManager::TitleMenuManager() {
            CreateMenu();
        }

        TitleMenuManager::~TitleMenuManager() {
            if (pressAnyButtonLayout_) delete pressAnyButtonLayout_;
            if (mainMenuLayout_) delete mainMenuLayout_;
            if (soundMenuLayout_) delete soundMenuLayout_; 
        }

        void TitleMenuManager::CreateMenu() {
            pressAnyButtonLayout_ = new app::ui::Layout();
            pressAnyButtonLayout_->Initialize<app::ui::TitleMenu>("Assets/ui/layout/titleLayout.json");

            mainMenuLayout_ = new app::ui::Layout();
            mainMenuLayout_->Initialize<app::ui::TitleSubMenu>("Assets/ui/layout/titleMenuLayout.json");

            // サウンドメニューのレイアウト初期化
            soundMenuLayout_ = new app::ui::Layout();
            soundMenuLayout_->Initialize<app::ui::SoundOptionMenu>("Assets/ui/layout/pauseMenuLayout.json");
        }

        void TitleMenuManager::Update() {
            if (pressAnyButtonLayout_) {
                pressAnyButtonLayout_->Update();
            }

            // ==========================================
            // 遷移アニメーション待機中の処理
            // ==========================================
            if (isPlayingAnimation_) {
                bool isAnimationFinished = true;

                // PressAnyButton中のワイプアニメーション更新
                if (currentState_ == TitleMenuState::enPressAnyButton) {
                    if (pressAnyButtonLayout_) pressAnyButtonLayout_->Update();
                }

                if (currentState_ == TitleMenuState::enMainMenu) {
                    if (mainMenuLayout_) mainMenuLayout_->Update();
                }
                else if (currentState_ == TitleMenuState::enSoundMenu) {
                    if (soundMenuLayout_) soundMenuLayout_->Update();
                }

                // 状態の切り替えが確定した瞬間
                if (isAnimationFinished) {
                    isPlayingAnimation_ = false;
                    currentState_ = nextState_;
                    if (currentState_ == TitleMenuState::enSoundMenu) {
                        auto* soundMenu = dynamic_cast<app::ui::SoundOptionMenu*>(soundMenuLayout_->GetMenu());
                        if (soundMenu) soundMenu->OnOpen();
                    }
                    else if (currentState_ == TitleMenuState::enMainMenu) {
                        auto* subMenu = dynamic_cast<app::ui::TitleSubMenu*>(mainMenuLayout_->GetMenu());
                        if (subMenu) subMenu->OnOpen();
                    }
                    else if (currentState_ == TitleMenuState::enPressAnyButton) {
                        auto* titleMenu = dynamic_cast<app::ui::TitleMenu*>(pressAnyButtonLayout_->GetMenu());
                        if (titleMenu) titleMenu->OnOpen();
                    }
                }
            }
            // ==========================================
            // 通常時の各メニュー処理
            // ==========================================
            else {
                switch (currentState_) {
                case TitleMenuState::enClosed:
                    break;

                case TitleMenuState::enPressAnyButton:
                {
                    if (g_pad[0]->IsTrigger(enButtonA)) {
                        auto* menu = dynamic_cast<app::ui::TitleMenu*>(pressAnyButtonLayout_->GetMenu());
                        if (menu) menu->OnClose();

                        nextState_ = TitleMenuState::enMainMenu;
                        isPlayingAnimation_ = true;
                    }
                    break;
                }

                case TitleMenuState::enMainMenu:
                {
                    if (mainMenuLayout_) {
                        mainMenuLayout_->Update();
                    }

                    auto* subMenu = dynamic_cast<app::ui::TitleSubMenu*>(mainMenuLayout_->GetMenu());
                    if (subMenu && subMenu->IsGameStartDecided()) {
                        isGameStartDecided_ = true;
                    }

                    if (g_pad[0]->IsTrigger(enButtonB)) {
                        app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));

                        if (subMenu) subMenu->OnClose();

                        nextState_ = TitleMenuState::enPressAnyButton;
                        isPlayingAnimation_ = true;
                    }
                    break;
                }

                case TitleMenuState::enSoundMenu:
                { 
                    if (soundMenuLayout_) {
                        soundMenuLayout_->Update(); // サウンドメニュー（カーソル移動など）の更新処理を実行
                    }

                    // Bボタンが押されたら、元のメニュー（MainMenu）に戻る
                    if (g_pad[0]->IsTrigger(enButtonB)) {
                        app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));

                        auto* soundMenu = dynamic_cast<app::ui::SoundOptionMenu*>(soundMenuLayout_->GetMenu());
                        if (soundMenu) soundMenu->OnClose(); // サウンドメニューを閉じる演出

                        nextState_ = TitleMenuState::enMainMenu;
                        isPlayingAnimation_ = true;
                    }
                    break;
                }
                } 
            } 
        }

        void TitleMenuManager::Render(RenderContext& rc) {
            // PressAnyButtonは常に描画するかどうか判断
            if (pressAnyButtonLayout_) {
                pressAnyButtonLayout_->Render(rc);
            }

            // JSONのUI描画後にスラッシュエフェクトを描画
            auto* titleMenu = dynamic_cast<app::ui::TitleMenu*>(pressAnyButtonLayout_->GetMenu());
            if (titleMenu) titleMenu->DrawSlash(rc);

            // 現在の状態に応じたメニューを描画
            if (currentState_ == TitleMenuState::enMainMenu) {
                if (mainMenuLayout_) {
                    mainMenuLayout_->Render(rc);
                }
            }
            else if (currentState_ == TitleMenuState::enSoundMenu) {
                if (soundMenuLayout_) {
                    soundMenuLayout_->Render(rc);
                }
            }
        }
    }
}