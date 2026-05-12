/**
 * TitleSubMenu.h
 */
#pragma once
#include "ui/Menu.h"

namespace app {
    namespace ui {
        class TitleSubMenu : public MenuBase {
        public:
            TitleSubMenu();
            virtual ~TitleSubMenu();

            void Update() override;
            void InitializeLogic() override;
            bool IsGameStartDecided() const { return isGameStartDecided_; }
            bool IsSystemDecided() const { return isSystemDecided_; }
            void ResetSystemDecided() { isSystemDecided_ = false; }

            void OnOpen();
            void OnClose();

        private:
            int currentIndex_ = 0;
            const int maxIndex_ = 2;
            bool isGameStartDecided_ = false;
            bool isSystemDecided_ = false;

            app::ui::UIIcon* highlight_ = nullptr;
        };
    }
}