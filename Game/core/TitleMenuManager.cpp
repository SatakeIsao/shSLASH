#include "stdafx.h"
#include "TitleMenuManager.h"
#include "ui/Layout.h"
#include "ui/Menu.h"
#include "ui/TitleMenu.h"   
#include "ui/TitleSubMenu.h"
#include "sound/SoundManager.h"
#include "ui/OptionMenu.h"

namespace app {
    namespace core {
        TitleMenuManager* TitleMenuManager::instance_ = nullptr;

        TitleMenuManager::TitleMenuManager() {
            CreateMenu();
        }

        TitleMenuManager::~TitleMenuManager() {
            if (pressAnyButtonLayout_) delete pressAnyButtonLayout_;
            if (mainMenuLayout_) delete mainMenuLayout_;
            if (optionMenuLayout_) delete optionMenuLayout_; 
        }

        void TitleMenuManager::CreateMenu() {
            pressAnyButtonLayout_ = new app::ui::Layout();
            pressAnyButtonLayout_->Initialize<app::ui::TitleMenu>("Assets/ui/layout/titleLayout.json");

            mainMenuLayout_ = new app::ui::Layout();
            mainMenuLayout_->Initialize<app::ui::TitleSubMenu>("Assets/ui/layout/titleMenuLayout.json");

            // サウンドメニューのレイアウト初期化
            optionMenuLayout_ = new app::ui::Layout();
            optionMenuLayout_->Initialize<app::ui::OptionMenu>("Assets/ui/layout/optionLayout.json");
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
                else if (currentState_ == TitleMenuState::enOptionMenu) {
                    if (optionMenuLayout_) optionMenuLayout_->Update();
                }

                // 状態の切り替えが確定した瞬間
                if (isAnimationFinished) {
                    isPlayingAnimation_ = false;
                    currentState_ = nextState_;
                    if (currentState_ == TitleMenuState::enOptionMenu) {
                        auto* optionMenu = dynamic_cast<app::ui::OptionMenu*>(optionMenuLayout_->GetMenu());
                        if (optionMenu) optionMenu->OnOpen();
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

                    else if (subMenu && subMenu->IsSystemDecided()) {
                        app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonDecision));

                        subMenu->ResetSystemDecided(); // フラグを元に戻す
                        subMenu->OnClose();            // メインメニューを閉じる演出

                        // マネージャー側で正しく遷移とアニメーションのフラグを立てる
                        nextState_ = TitleMenuState::enOptionMenu;
                        isPlayingAnimation_ = true; // ★これがtrueになることで、OptionMenuのOnOpen()が呼ばれて画面に表示される！
                    }

                    if (g_pad[0]->IsTrigger(enButtonB)) {
                        app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonReturn));

                        if (subMenu) subMenu->OnClose();

                        nextState_ = TitleMenuState::enPressAnyButton;
                        isPlayingAnimation_ = true;
                    }
                    break;
                }

                case TitleMenuState::enOptionMenu:
                { 
                    if (optionMenuLayout_) {
                        optionMenuLayout_->Update(); // サウンドメニュー（カーソル移動など）の更新処理を実行
                    }

                    // Bボタンが押されたら、元のメニュー（MainMenu）に戻る
                    if (g_pad[0]->IsTrigger(enButtonB)) {
                        app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonReturn));

                        auto* optionMenu = dynamic_cast<app::ui::OptionMenu*>(optionMenuLayout_->GetMenu());
                        if (optionMenu) optionMenu->OnClose(); // サウンドメニューを閉じる演出

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
            else if (currentState_ == TitleMenuState::enOptionMenu) {
                if (optionMenuLayout_) {
                    optionMenuLayout_->Render(rc);
                }
            }
        }
    }
}