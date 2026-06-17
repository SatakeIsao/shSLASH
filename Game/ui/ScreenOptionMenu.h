#pragma once
#include "Menu.h"

namespace app
{
    namespace ui
    {
        class ScreenOptionMenu : public MenuBase
        {
        public:
            ScreenOptionMenu();
            virtual ~ScreenOptionMenu() = default;

            void InitializeLogic() override;
            void Update() override;

            void OnOpen();
            void OnClose();
            void SetDraw(bool isDraw);

        private:
            static const int MAX_ITEMS = 2;

            int cursorIndex_ = 0;

            bool isMotionBlur_ = true;
            bool isDepthOfField_ = true;
        };
    }
}
