#include "stdafx.h"
#include "OptionMenu.h"
#include "ui/SoundOptionMenu.h" 
#include "ui/CameraOptionMenu.h"
#include "ui/ReturnToTitleMenu.h"
#include "sound/SoundManager.h"

namespace app
{
    namespace ui
    {
        OptionMenu::OptionMenu() {}
        OptionMenu::~OptionMenu() {}

        void OptionMenu::InitializeLogic()
        {
            iconBases_[0] = GetUI<app::ui::UIIcon>(Hash32("IconBase_1"));
            iconBases_[1] = GetUI<app::ui::UIIcon>(Hash32("IconBase_2"));
            iconBases_[2] = GetUI<app::ui::UIIcon>(Hash32("IconBase_3"));

            icons_[0] = GetUI<app::ui::UIIcon>(Hash32("Icon_Vol"));
            icons_[1] = GetUI<app::ui::UIIcon>(Hash32("Icon_Camera"));
            icons_[2] = GetUI<app::ui::UIIcon>(Hash32("Icon_Return"));

            buttonLB_ = GetUI<app::ui::UIIcon>(Hash32("BUTTUN_LB"));
            buttonRB_ = GetUI<app::ui::UIIcon>(Hash32("BUTTUN_RB"));

            soundLayout_ = std::make_unique<app::ui::Layout>();
            soundLayout_->Initialize<app::ui::SoundOptionMenu>("Assets/ui/layout/pauseMenuLayout.json");

            cameraLayout_ = std::make_unique<app::ui::Layout>();
            cameraLayout_->Initialize<app::ui::CameraOptionMenu>("Assets/ui/layout/cameraMenuLayout.json");

            titleLayout_ = std::make_unique<app::ui::Layout>();
            titleLayout_->Initialize<app::ui::ReturnToTitleMenu>("Assets/ui/layout/returnToTitleMenuLayout.json");

            // サウンドタブを選択
            ChangeTab(Tab::Sound);
        }

        void OptionMenu::OnOpen()
        {
            auto* canvas = GetCanvas();
            if (canvas) canvas->isDraw = true;

            // 画面を開いたときは、現在選択中のタブの表示状態を同期させる
            ChangeTab(currentTab_);
        }

        void OptionMenu::OnClose()
        {
            auto* canvas = GetCanvas();
            if (canvas) canvas->isDraw = false;

            // 基盤が閉じるときは子レイアウトも確実に非表示にする
            if (soundLayout_ && soundLayout_->GetMenu())  dynamic_cast<SoundOptionMenu*>(soundLayout_->GetMenu())->SetDraw(false);
            if (cameraLayout_ && cameraLayout_->GetMenu()) dynamic_cast<CameraOptionMenu*>(cameraLayout_->GetMenu())->SetDraw(false);
            if (titleLayout_ && titleLayout_->GetMenu()) dynamic_cast<ReturnToTitleMenu*>(titleLayout_->GetMenu())->SetDraw(false);
        }

        void OptionMenu::ChangeTab(Tab nextTab)
        {
            currentTab_ = nextTab;

            // 子クラスのポインタをダイナミックキャストで取得
            auto* soundMenu = soundLayout_ ? dynamic_cast<SoundOptionMenu*>(soundLayout_->GetMenu()) : nullptr;
            auto* cameraMenu = cameraLayout_ ? dynamic_cast<CameraOptionMenu*>(cameraLayout_->GetMenu()) : nullptr;
            auto* titleMenu = titleLayout_ ? dynamic_cast<ReturnToTitleMenu*>(titleLayout_->GetMenu()) : nullptr;

            // 切り替え
            if (soundMenu)  soundMenu->SetDraw(currentTab_ == Tab::Sound);
            if (cameraMenu) cameraMenu->SetDraw(currentTab_ == Tab::Camera);
            if (titleMenu) titleMenu->SetDraw(currentTab_ == Tab::Title);

            for (int i = 0; i < static_cast<int>(Tab::Max); ++i)
            {
                bool isSelected = (i == static_cast<int>(currentTab_));

                // 土台の移動
                if (iconBases_[i]) {
                    float baseX = iconBases_[i]->transform.localPosition.x;
                    float baseZ = iconBases_[i]->transform.localPosition.z;
                    // 選ばれているタブだけ上にずらす
                    float targetY = isSelected ? 145.0f : 125.0f;

                    iconBases_[i]->transform.localPosition = Vector3(baseX, targetY, baseZ);
                    iconBases_[i]->transform.UpdateTransform();
                }

                // アイコンの移動
                if (icons_[i]) {
                    float iconX = icons_[i]->transform.localPosition.x;
                    float iconZ = icons_[i]->transform.localPosition.z;
                    // 土台と同じく上にずらす
                    float targetY = isSelected ? 154.0f : 134.0f;

                    icons_[i]->transform.localPosition = Vector3(iconX, targetY, iconZ);
                    icons_[i]->transform.UpdateTransform();
                }
            }

        }

        void OptionMenu::Update()
        {
            //[TODO]：アニメーションなどはここで
            MenuBase::Update();
            if (soundLayout_)soundLayout_->Update();
            if (cameraLayout_)cameraLayout_->Update();
            if (titleLayout_)titleLayout_->Update();

            auto* canvas = GetCanvas();
            if (!canvas || !canvas->isDraw) return;

            if (buttonLB_) {
                // IsPress などの「押している間」の判定を使用します
                if (g_pad[0]->IsPress(enButtonLB1)) {
                    buttonLB_->color.x = 1.0f; // R
                    buttonLB_->color.y = 1.0f; // G
                    buttonLB_->color.z = 0.0f; // B (黄色)
                }
                else {
                    buttonLB_->color.x = 1.0f; // R
                    buttonLB_->color.y = 1.0f; // G
                    buttonLB_->color.z = 1.0f; // B (白に戻す)
                }
            }

            if (buttonRB_) {
                if (g_pad[0]->IsPress(enButtonRB1)) {
                    buttonRB_->color.x = 1.0f;
                    buttonRB_->color.y = 1.0f;
                    buttonRB_->color.z = 0.0f; // 黄色
                }
                else {
                    buttonRB_->color.x = 1.0f;
                    buttonRB_->color.y = 1.0f;
                    buttonRB_->color.z = 1.0f; // 白に戻す
                }
            }

            // ==========================================
            //　LB/RBボタンによるタブ切り替え処理
            // ==========================================
            if (g_pad[0]->IsTrigger(enButtonLB1))
            {
                app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
                int next = static_cast<int>(currentTab_) - 1;
                if (next < 0) next = static_cast<int>(Tab::Max) - 1;
                ChangeTab(static_cast<Tab>(next));
            }
            if (g_pad[0]->IsTrigger(enButtonRB1))
            {
                app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
                int next = static_cast<int>(currentTab_) + 1;
                if (next >= static_cast<int>(Tab::Max)) next = 0;
                ChangeTab(static_cast<Tab>(next));
            }
        }

        void OptionMenu::Render(RenderContext& rc)
        {
            auto* canvas = GetCanvas();
            if (!canvas || !canvas->isDraw) return;

            MenuBase::Render(rc);

            // 現在選択されているタブの設定項目を上に重ねて描画
            if (currentTab_ == Tab::Sound && soundLayout_)
            {
                soundLayout_->Render(rc);
            }
            else if (currentTab_ == Tab::Camera && cameraLayout_)
            {
                cameraLayout_->Render(rc);
            }
            else if (currentTab_ == Tab::Title && titleLayout_)
            {
                titleLayout_->Render(rc);
            }
        }

        bool OptionMenu::IsReturnTitleDecided() const {
            if (!titleLayout_) return false;
            auto* menu = dynamic_cast<ReturnToTitleMenu*>(titleLayout_->GetMenu());
            return menu ? menu->IsReturnTitleDecided() : false;
        }

        bool OptionMenu::IsRetryDecided() const {
            if (!titleLayout_) return false;
            auto* menu = dynamic_cast<ReturnToTitleMenu*>(titleLayout_->GetMenu());
            return menu ? menu->IsRetryDecided() : false;
        }

        void OptionMenu::ResetTitleFlags() {
            if (!titleLayout_) return;
            auto* menu = dynamic_cast<ReturnToTitleMenu*>(titleLayout_->GetMenu());
            if (menu) menu->ResetFlags();
        }
    }
}