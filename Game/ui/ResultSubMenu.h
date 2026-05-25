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
            // 今回操作したいUIのポインタ
            UIIcon* backFilter_ = nullptr;
            UIIcon* menuLine1_ = nullptr;
            UIIcon* menuLine2_ = nullptr;
            UIIcon* menuBase_ = nullptr;
            UIIcon* retry_ = nullptr;
            UIIcon* title_ = nullptr;
            UIIcon* exit_ = nullptr;
            UIIcon* highlight_ = nullptr;
        };
    }
}