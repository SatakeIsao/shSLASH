#pragma once
#include "ui/Menu.h"
#include "ui/Layout.h"

namespace app
{
    namespace ui
    {
        class OptionMenu : public MenuBase
        {
        public:
            OptionMenu();
            virtual ~OptionMenu();

            void InitializeLogic() override;
            void Update() override;
            void Render(RenderContext& rc) override; 

            void OnOpen();
            void OnClose();

            bool IsReturnTitleDecided() const;
            bool IsRetryDecided() const;
            void ResetTitleFlags();

            bool IsPause() const { return false; } // 消さずに切り替えるため常にfalse

        private:
            // タブの種類を定義
            enum class Tab
            {
                Sound,
                Camera,
                Screen,
                Title,
                Max
            };

            Tab currentTab_ = Tab::Sound;

            UIIcon* iconBases_[4] = { nullptr };
            UIIcon* icons_[4] = { nullptr };

            UIIcon* buttonLB_ = nullptr;
            UIIcon* buttonRB_ = nullptr;

            // 基盤クラスが子要素として各設定画面のレイアウトを自動管理する
            std::unique_ptr<app::ui::Layout> soundLayout_;
            std::unique_ptr<app::ui::Layout> cameraLayout_;
            std::unique_ptr<app::ui::Layout> screenLayout_;
            std::unique_ptr<app::ui::Layout> titleLayout_;

            // タブ切り替え用の内部関数
            void ChangeTab(Tab nextTab);
        };
    }
}