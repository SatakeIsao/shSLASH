#include "stdafx.h"
#include "PauseManager.h"
#include "memory.h"
#include "ui/SoundOptionMenu.h"
#include "ui/Layout.h"


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
        }

        void PauseManager::Update()
        {
            isPauseTrigger = false;
            app::ui::SoundOptionMenu* menu = dynamic_cast<app::ui::SoundOptionMenu*>(layout_->GetMenu());

            // ここでボタンを押す処理書いて
            bool isOldPause = isPause_;
            if (canPause_
                &&g_pad[0]->IsTrigger(enButtonStart))
            {
                if (!isPause_)
                {
                    // 開く処理
                    isPause_ = true;
                    isPauseTrigger = true;
                    if (menu) menu->OnOpen();
                }
                else
                {
                    // 閉じる処理
                    if (menu) menu->OnClose();
                }
            }

            // Pause中じゃなければ処理をしない
            if (!isPause_) {
                return;
            }

#if defined(APP_DEBUGGABLE_PAUSE_MENU_CREATE)
            if (isPause_) {
                if (layout_ == nullptr) {
                    CreateMenu();
                }
            } else {
                if (layout_) {
                    // 削除
                    delete layout_;
                    layout_ = nullptr;
                }
            }
#else
            // Pauseが切り替わった
            if (isOldPause != isPause_) {
                app::ui::SoundOptionMenu* menu = dynamic_cast<app::ui::SoundOptionMenu*>(layout_->GetMenu());
                // 開く
                if (isPause_) {
                    menu->OnOpen();
                } else {
                    menu->OnClose();
                }
            }
#endif
            if (!isPause_) {
                return;
            }

            if (layout_) {
                layout_->Update();
            }

            if (menu && !menu->IsPause())
            {
                isPause_ = false;
            }
        }


        void PauseManager::Render(RenderContext& rc)
        {
            if (!isPause_) {
                return;
            }

            if (layout_){
                layout_->Render(rc);
            }
        }


        void PauseManager::CreateMenu()
        {
            layout_ = new app::ui::Layout();
            layout_->Initialize<app::ui::SoundOptionMenu>("Assets/ui/layout/pauseMenuLayout.json");
        }
    }
}
