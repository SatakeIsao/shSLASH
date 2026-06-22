#pragma once
#include "Menu.h"

namespace app
{
    namespace ui
    {
        class TutorialReturnConfirmMenu : public MenuBase
        {
        private:
            UIIcon* yesIcon_ = nullptr;
            UIIcon* noIcon_  = nullptr;
            UIIcon* cursor_  = nullptr;

            int currentIndex_ = 1; // 0=YES 1=NO

            bool isYesDecided_ = false;
            bool isNoDecided_  = false;

        public:
            TutorialReturnConfirmMenu()  = default;
            ~TutorialReturnConfirmMenu() = default;

            void InitializeLogic() override;
            void Update() override;

            bool IsYesDecided() const;
            bool IsNoDecided()  const;

            void ResetState();

        private:
            void ApplyCursor();
        };
    }
}
