#include "stdafx.h"
#include "PauseManager.h"
#include "ui/OptionMenu.h"
#include "ui/PauseMenu.h"
#include "ui/Layout.h"
#include "sound/SoundManager.h"



/** デバッグ用処理 */
#if defined(_DEBUG)
//#define APP_DEBUGGABLE_PAUSE_MENU_CREATE
#endif


namespace app
{
    namespace core
    {
        PauseManager* PauseManager::instance_ = nullptr; //初期化


        PauseManager::PauseManager()
        {
            CreateMenu();
        }

        PauseManager::~PauseManager()
        {
            if (layout_)
            {
                delete layout_;
                layout_ = nullptr;
            }
            if (pauseLayout_)
            {
                delete pauseLayout_;
                pauseLayout_ = nullptr;
            }
        }

        void PauseManager::Update()
        {
            app::ui::OptionMenu* menu = dynamic_cast<app::ui::OptionMenu*>(layout_->GetMenu());
            app::ui::PauseMenu* pause = dynamic_cast<app::ui::PauseMenu*>(pauseLayout_->GetMenu());

            if (isPlayingAnimation_)
            {
                bool isAnimationFinished = false;

                // 今閉じようとしている画面の Update だけ回して、アニメーションを進める
                switch (currentState_)
                {
                case PauseState::enPause:
                    if (pauseLayout_)
                    {
                        pauseLayout_->Update();
                    }
                    if (pause && !pause->IsPause())
                    {
                        isAnimationFinished = true;
                    }
                    break;

                case PauseState::enOption:
                    if (layout_)
                    {
                        layout_->Update();
                    }
                    if (menu && !menu->IsPause())
                    {
                        isAnimationFinished = true;
                    }
                    break;

                case PauseState::enClosed:
                    isAnimationFinished = true;
                    break;
                }

                // アニメーションが終わったら次の画面に遷移。
                if (isAnimationFinished)
                {
                    currentState_ = nextState_;
                    isPlayingAnimation_ = false;

                    // 新しい画面を開く
                    switch (currentState_)
                    {
                    case PauseState::enPause:
                        if (pause)
                        {
                            pause->OnOpen();
                        }
                        break;

                    case PauseState::enOption:
                        if (menu)
                        {
                            menu->OnOpen();
                        }
                        break;


                    case PauseState::enClosed:
                        isPause_ = false; // 完全にポーズが終了した
                        break;
                    }
                }
                return;
            }

            if (canPause_ &&
                g_pad[0]->IsTrigger(enButtonStart))
            {
                app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
                if (currentState_ == PauseState::enClosed)
                {
                    // ポーズを開く処理
                    isPause_ = true;
                    currentState_ = PauseState::enPause;
                    if (pause) pause->OnOpen();
                }
                else
                {
                    // どの画面にいても、STARTを押したら閉じる予約をする
                    if (currentState_ == PauseState::enPause && pause)
                    {
                        pause->OnClose();
                    }
                    if (currentState_ == PauseState::enOption && menu)
                    {
                        menu->OnClose();
                    }

                    nextState_ = PauseState::enClosed;
                    isPlayingAnimation_ = true;
                }
            }

            // Pause中じゃなければ処理をしない
            if (currentState_ == PauseState::enClosed) {
                return;
            }

            // 現在の画面ごとの更新処理
            switch (currentState_) {
            case PauseState::enPause:
                if (pauseLayout_) pauseLayout_->Update();

                if (g_pad[0]->IsTrigger(enButtonY))
                {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
                    if (pause) pause->OnClose();
                    nextState_ = PauseState::enOption;
                    isPlayingAnimation_ = true;
                }
                break;

            case PauseState::enOption:
                if (layout_) layout_->Update();


                // Bボタンでメインのポーズメニューへ
                if (g_pad[0]->IsTrigger(enButtonB))
                {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
                    menu->OnClose();
                    nextState_ = PauseState::enPause;
                    isPlayingAnimation_ = true;
                }
                break;
            }
        }

        bool PauseManager::IsReturnToTitleRequested() const
        {
            if (!layout_) return false;
            auto* menu = dynamic_cast<app::ui::OptionMenu*>(layout_->GetMenu());
            return menu ? menu->IsReturnTitleDecided() : false;
        }

        bool PauseManager::IsRetryRequested() const
        {
            if (!layout_) return false;
            auto* menu = dynamic_cast<app::ui::OptionMenu*>(layout_->GetMenu());
            return menu ? menu->IsRetryDecided() : false;
        }

        void PauseManager::Render(RenderContext& rc)
        {
            // 閉じていたら描画しない
            if (currentState_ == PauseState::enClosed)
            {
                return;
            }

            // 状態に応じて描画するレイアウトを切り替える
            switch (currentState_) {
            case PauseState::enPause:
                if (pauseLayout_)
                {
                    pauseLayout_->Render(rc);
                }
                break;

            case PauseState::enOption:
                if (layout_)
                {
                    layout_->Render(rc);
                }
                break;
            }
        }


        void PauseManager::CreateMenu()
        {
            /** プションレイアウト */
            {
                layout_ = new app::ui::Layout();
                layout_->Initialize<app::ui::OptionMenu>("Assets/ui/layout/optionLayout.json");
            }
            /** ポーズレイアウト */
            {
                pauseLayout_ = new app::ui::Layout();
                pauseLayout_->Initialize<app::ui::PauseMenu>("Assets/ui/layout/pauseLayout.json");
            }
        }
    }
}
