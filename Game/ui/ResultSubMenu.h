#pragma once
#include "ui/Menu.h"

namespace app {
    namespace ui {
        class ResultSubMenu : public MenuBase {
        public:
            ResultSubMenu();
            virtual ~ResultSubMenu();

            void InitializeLogic() override;
            void Update() override;
            void OnOpen(); 

        private:
            void StartFadeIn();
            // 今回操作したいUIのポインタ
            UIIcon* backFilter_ = nullptr;
            UIIcon* menuLine1_ = nullptr;
            UIIcon* menuLine2_ = nullptr;
            UIIcon* menuBase_ = nullptr;
            UIIcon* retry_ = nullptr;
            UIIcon* title_ = nullptr;
            UIIcon* exit_ = nullptr;
            UIIcon* highlight_ = nullptr;

            int cursorIndex_ = 0;
            bool isReturnTitleDecided_ = false;
            bool isRetryDecided_ = false;
            bool isExitDecided_ = false;

            int waitFrame_ = 0;
            float fadeProgress_ = 0.0f;

            Vector3 cursorPositions_[3];
        public:
            bool IsReturnTitleDecided() const { return isReturnTitleDecided_; }
            bool IsRetryDecided() const { return isRetryDecided_; }
            bool IsExitDecided() const { return isExitDecided_; }
        };
    }
}