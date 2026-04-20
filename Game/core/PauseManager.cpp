#include "stdafx.h"
#include "PauseManager.h"
#include "ui/SoundOptionMenu.h"
#include "ui/PauseMenu.h"
#include "ui/ReturnToTitleMenu.h"
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
            if (returnToTitleLayout_)
            {
                delete returnToTitleLayout_;
                returnToTitleLayout_ = nullptr;
            }
        }

        void PauseManager::Update()
        {
            app::ui::SoundOptionMenu* menu = dynamic_cast<app::ui::SoundOptionMenu*>(layout_->GetMenu());
            app::ui::PauseMenu* pause = dynamic_cast<app::ui::PauseMenu*>(pauseLayout_->GetMenu());
            title_ = dynamic_cast<app::ui::ReturnToTitleMenu*>(returnToTitleLayout_->GetMenu());

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

                case PauseState::enSound:
                    if (layout_)
                    {
                        layout_->Update();
                    }
                    if (menu && !menu->IsPause())
                    {
                        isAnimationFinished = true;
                    }
                    break;

                case PauseState::enTitle:
                    if (returnToTitleLayout_)
                    {
                        returnToTitleLayout_->Update();
                    }
                    if (title_ && !title_->IsPause())
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

                    case PauseState::enSound:
                        if (menu)
                        {
                            menu->OnOpen();
                        }
                        break;

                    case PauseState::enTitle:
                        if (title_)
                        {
                            title_->OnOpen();
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
                    if (currentState_ == PauseState::enSound && menu)
                    {
                        menu->OnClose();
                    }
                    if (currentState_ == PauseState::enTitle && title_)
                    {
                        title_->OnClose();
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

                // Aボタン&& インデックスが0のとき
                if (g_pad[0]->IsTrigger(enButtonA)
                    && pause->GerCurrentIndex() == 0)
                {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
                    pause->OnClose();
                    nextState_ = PauseState::enSound;
                    isPlayingAnimation_ = true;
                }
                // Aボタン&& インデックスが1の時
                else if (g_pad[0]->IsTrigger(enButtonA)
                    && pause->GerCurrentIndex() == 1)
                {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
                    pause->OnClose();
                    nextState_ = PauseState::enTitle;
                    isPlayingAnimation_ = true;
                }
                break;

            case PauseState::enSound:
                if (layout_) layout_->Update();

                // Bボタンでメインのポーズメニューへ「戻る」
                if (g_pad[0]->IsTrigger(enButtonB))
                {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
                    menu->OnClose();
                    nextState_ = PauseState::enPause;
                    isPlayingAnimation_ = true;
                }
                break;

            case PauseState::enTitle:
                if (returnToTitleLayout_) returnToTitleLayout_->Update();
                // ポーズ画面に遷移。
                if (title_->IsDecidedNo())
                {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
                    title_->OnClose();
                    nextState_ = PauseState::enPause;
                    isPlayingAnimation_ = true;
                }
                /** タイトル */
                if (title_->IsDecidedYes())
                {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
                    title_->OnClose();
                    nextState_ = PauseState::enClosed;
                    isPlayingAnimation_ = true;
                }
                break;
            }
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

            case PauseState::enSound:
                if (layout_)
                {
                    layout_->Render(rc);
                }
                break;

            case PauseState::enTitle:
               if (returnToTitleLayout_)
               {
                   returnToTitleLayout_->Render(rc);
               }
                break;
            }
        }


        void PauseManager::CreateMenu()
        {
            /** サウンドオプションレイアウト */
            {
                layout_ = new app::ui::Layout();
                layout_->Initialize<app::ui::SoundOptionMenu>("Assets/ui/layout/pauseMenuLayout.json");
            }
            /** ポーズレイアウト */
            {
                pauseLayout_ = new app::ui::Layout();
                pauseLayout_->Initialize<app::ui::PauseMenu>("Assets/ui/layout/pauseLayout.json");
            }
            /** タイトルに戻るレイアウト */
            {
                returnToTitleLayout_ = new app::ui::Layout();
                returnToTitleLayout_->Initialize<app::ui::ReturnToTitleMenu>("Assets/ui/layout/returnToTitleMenuLayout.json");
            }
        }
        bool PauseManager::IsReturnToTitleRequested() const
        {
            if (title_
                && title_->IsDecidedYes())
            {
                return true;
            }
            return false;
        }
    }
}
