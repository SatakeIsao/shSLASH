#include "stdafx.h"
#include "ReturnToTitleMenu.h"
#include "sound/SoundManager.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"


namespace app
{
	namespace ui
	{
		ReturnToTitleMenu::ReturnToTitleMenu() {}
		ReturnToTitleMenu::~ReturnToTitleMenu() {}

        void ReturnToTitleMenu::InitializeLogic() {
            // JSONからアニメーションをアタッチ
            cursol_ = GetUI<UIIcon>(Hash32("Cursol"));
            if (cursol_) {
                app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(cursol_, Hash32("FadeIn"));
            }
        }

        void ReturnToTitleMenu::Update() {
            MenuBase::Update();

            auto* canvas = GetCanvas();
            if (!canvas || !canvas->isDraw) return;

            if (g_pad[0]->IsTrigger(enButtonUp)) {
                app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
                currentIndex_--;
                if (currentIndex_ < 0) currentIndex_ = maxIndex_;
            }
            else if (g_pad[0]->IsTrigger(enButtonDown)) {
                app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
                currentIndex_++;
                if (currentIndex_ > maxIndex_) currentIndex_ = 0;
            }

            // JSONの座標に合わせてカーソルを移動
            if (cursol_) {
                if (currentIndex_ == 0) cursol_->transform.localPosition.y = -45.0f;
                else if (currentIndex_ == 1) cursol_->transform.localPosition.y = -225.0f;
            }

            // Aボタンで決定処理
            if (g_pad[0]->IsTrigger(enButtonA)) {
                app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
                switch (currentIndex_) {
                case 0:
                    isReturnTitleDecided_ = true;
                    break;
                case 1:
                    isRetryDecided_ = true;
                    break;
                }
            }
        }

        void ReturnToTitleMenu::SetDraw(bool isDraw) {
            auto* canvas = GetCanvas();
            if (canvas) canvas->isDraw = isDraw;

            if (isDraw) {
                ResetFlags();
                currentIndex_ = 0; // 開くたびにカーソルを上に戻す
                if (cursol_) {
                    cursol_->transform.localPosition.y = -35.0f;
                    auto* anim = cursol_->FindAnimation(Hash32("FadeIn"));
                    if (anim) anim->Play();
                }
            }
            else {
                if (cursol_) cursol_->StopSpriteAnimation();
            }
        }
	}
}

