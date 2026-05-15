#pragma once
#include "ui/Menu.h"

namespace app {
    namespace ui {
        class TitleMenu : public MenuBase {
            // アニメーションの状態管理用
            enum class AnimState {
                None,
                Opening_Step1, // 少し開く
                Opening_Step2,
                ShakeStep1,   // 衝撃上跳ね
                ShakeStep2,   // 衝撃戻り
                ShakeStep3,
                ShakeStep4,
                /** ワイプ中の衝撃シェイク */
                WipeShakeStep1,
                WipeShakeStep2,
                WipeShakeStep3,
                WipeShakeStep4,
                Closing,
                SlashClosing
            };

            // ワイプパラメータ構造体
                struct WipeParam {
                Vector2 wipeDir;
                float wipeSize;
            };


        public:
            void InitializeLogic() override;
            void Update() override;
            void OnOpen();
            void OnClose();
            bool IsPlaying();
            void DrawSlash(RenderContext& rc) {
                if (isSlashVisible_) {
                    slashSpriteRender_.Draw(rc);
                }
            }

        private:
            app::ui::UIIcon* pressA_ = nullptr;
            app::ui::UIIcon* titleLeft_ = nullptr;
            app::ui::UIIcon* titleRight_ = nullptr;
            app::ui::UIIcon* slashedEffect_ = nullptr;

            AnimState animState_ = AnimState::None;
            float animTimer_ = 0.0f;
            // ワイプパラメータ
            WipeParam wipeParam_;
            bool isWiping_ = false;
            bool isSlashVisible_ = false;
            SpriteRender slashSpriteRender_;
            float wipeTimer_ = 0.0f;
            float wipeDuration_ = 0.5f;
        };
    }
}