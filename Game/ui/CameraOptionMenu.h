#pragma once
#include "Menu.h"

namespace app
{
    namespace ui
    {
        class CameraOptionMenu : public MenuBase
        {
        public:
            CameraOptionMenu();
            virtual ~CameraOptionMenu();

            void InitializeLogic() override;
            void Update() override;

            void OnOpen();
            void OnClose();

            // 表示/非表示を直接切り替える関数
            void SetDraw(bool isDraw);

        private:
            // 項目数（X軸リバース, Y軸リバースの2つ）
            static const int MAX_ITEMS = 3;

            int cursorIndex_ = 0;

            // 現在の設定状態（false = ノーマル, true = リバース）
            bool isXReverse_ = false;
            bool isYReverse_ = false;
        };
    }
}