#pragma once
#include "ui/Menu.h"

namespace app {
    namespace ui {
        class TitleMenu : public MenuBase {
            // アニメーションの状態管理用
            enum class AnimState {
                None,
                Opening_Step1, // 少し開く
                Closing
            };

        public:
            void InitializeLogic() override;
            void Update() override;
            void OnOpen();
            void OnClose();
            bool IsPlaying();

        private:
            app::ui::UIIcon* pressA_ = nullptr;
            app::ui::UIIcon* titleLeft_ = nullptr;
            app::ui::UIIcon* titleRight_ = nullptr;

            AnimState animState_ = AnimState::None;
            float animTimer_ = 0.0f;
        };
    }
}