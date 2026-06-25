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
            bool IsTutorialDecided() const { return isTutorialDecided_; }
            bool IsExitDecided() const { return isExitDecided_; }
            void ResetSystemDecided() { isSystemDecided_ = false; }
            void ResetTutorialDecided() { isTutorialDecided_ = false; }
            void ResetExitDecided() { isExitDecided_ = false; }

            void OnOpen();
            void OnClose();

        private:
            int currentIndex_ = 0;
            const int maxIndex_ = 3;
            bool isGameStartDecided_ = false;
            bool isSystemDecided_ = false;
            bool isTutorialDecided_ = false;
            bool isExitDecided_ = false;

            app::ui::UIIcon* highlight_ = nullptr;
        };
    }
}