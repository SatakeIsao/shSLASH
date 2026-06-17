#include "stdafx.h"
#include "ScreenOptionMenu.h"
#include "sound/SoundManager.h"
#include "camera/CameraManager.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"

namespace
{
    constexpr float CURSOR_X[2] = { 165.0f, 165.0f };
    constexpr float CURSOR_Y[2] = { 15.0f, -78.0f };
    constexpr float HIGHLIGHT_X[2] = { -280.0f, -280.0f };
    constexpr float HIGHLIGHT_Y[2] = { 10.0f, -83.0f };
}

namespace app
{
    namespace ui
    {
        ScreenOptionMenu::ScreenOptionMenu()
        {
        }

        void ScreenOptionMenu::InitializeLogic()
        {
            auto* canvas = GetCanvas();
            if (canvas)
            {
                canvas->transform.localScale = Vector3::One;
                canvas->transform.UpdateTransform();
            }

            ForEachUI([](app::ui::UIBase* ui) {
                ui->color.w = 1.0f;
                ui->isDraw = true;
            });

            cursorIndex_ = 0;
            isMotionBlur_ = app::camera::CameraManager::Get().IsMotionBlurEnabled();
            isDepthOfField_ = app::camera::CameraManager::Get().IsDepthOfFieldEnabled();

            auto* mbOn  = GetUI<app::ui::UIIcon>(Hash32("Word_MotionBlur_ON"));
            auto* mbOff = GetUI<app::ui::UIIcon>(Hash32("Word_MotionBlur_OFF"));
            if (mbOn)  mbOn->isDraw  = isMotionBlur_;
            if (mbOff) mbOff->isDraw = !isMotionBlur_;

            auto* dofOn  = GetUI<app::ui::UIIcon>(Hash32("Word_DepthOfField_ON"));
            auto* dofOff = GetUI<app::ui::UIIcon>(Hash32("Word_DepthOfField_OFF"));
            if (dofOn)  dofOn->isDraw  = isDepthOfField_;
            if (dofOff) dofOff->isDraw = !isDepthOfField_;

            auto* highlight = GetUI<app::ui::UIIcon>(Hash32("Highlight"));
            if (highlight)
            {
                app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(highlight, Hash32("FadeIn"));
            }
        }

        void ScreenOptionMenu::OnOpen()
        {
            SetDraw(true);
        }

        void ScreenOptionMenu::OnClose()
        {
            SetDraw(false);
        }

        void ScreenOptionMenu::SetDraw(bool isDraw)
        {
            auto* canvas = GetCanvas();
            if (canvas) canvas->isDraw = isDraw;

            auto* highlight = GetUI<app::ui::UIIcon>(Hash32("Highlight"));
            if (isDraw)
            {
                isMotionBlur_ = app::camera::CameraManager::Get().IsMotionBlurEnabled();
                isDepthOfField_ = app::camera::CameraManager::Get().IsDepthOfFieldEnabled();

                auto* mbOn  = GetUI<app::ui::UIIcon>(Hash32("Word_MotionBlur_ON"));
                auto* mbOff = GetUI<app::ui::UIIcon>(Hash32("Word_MotionBlur_OFF"));
                if (mbOn)  mbOn->isDraw  = isMotionBlur_;
                if (mbOff) mbOff->isDraw = !isMotionBlur_;

                auto* dofOn  = GetUI<app::ui::UIIcon>(Hash32("Word_DepthOfField_ON"));
                auto* dofOff = GetUI<app::ui::UIIcon>(Hash32("Word_DepthOfField_OFF"));
                if (dofOn)  dofOn->isDraw  = isDepthOfField_;
                if (dofOff) dofOff->isDraw = !isDepthOfField_;

                if (highlight)
                {
                    auto* anim = highlight->FindAnimation(Hash32("FadeIn"));
                    if (anim) anim->Play();
                }
            }
            else
            {
                if (highlight) highlight->StopSpriteAnimation();
            }
        }

        void ScreenOptionMenu::Update()
        {
            auto* canvas = GetCanvas();
            if (!canvas || !canvas->isDraw) return;

            MenuBase::Update();

            bool isSettingsChanged = false;

            if (g_pad[0]->IsTrigger(enButtonDown))
            {
                app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonMove));
                cursorIndex_++;
                if (cursorIndex_ >= MAX_ITEMS) cursorIndex_ = 0;
            }
            else if (g_pad[0]->IsTrigger(enButtonUp))
            {
                app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonMove));
                cursorIndex_--;
                if (cursorIndex_ < 0) cursorIndex_ = MAX_ITEMS - 1;
            }

            auto* cursor = GetUI<app::ui::UIIcon>(Hash32("Cursor"));
            if (cursor)
            {
                cursor->transform.localPosition.x = CURSOR_X[cursorIndex_];
                cursor->transform.localPosition.y = CURSOR_Y[cursorIndex_];
                cursor->transform.UpdateTransform();
            }

            auto* highlight = GetUI<app::ui::UIIcon>(Hash32("Highlight"));
            if (highlight)
            {
                highlight->transform.localPosition.x = HIGHLIGHT_X[cursorIndex_];
                highlight->transform.localPosition.y = HIGHLIGHT_Y[cursorIndex_];
                highlight->transform.UpdateTransform();
            }

            if (cursorIndex_ == 0)
            {
                if (g_pad[0]->IsTrigger(enButtonLeft) || g_pad[0]->IsTrigger(enButtonRight))
                {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonMove));
                    isMotionBlur_ = !isMotionBlur_;
                    isSettingsChanged = true;
                }
            }
            else if (cursorIndex_ == 1)
            {
                if (g_pad[0]->IsTrigger(enButtonLeft) || g_pad[0]->IsTrigger(enButtonRight))
                {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonMove));
                    isDepthOfField_ = !isDepthOfField_;
                    isSettingsChanged = true;
                }
            }

            if (g_pad[0]->IsTrigger(enButtonY))
            {
                app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonReset));
                isMotionBlur_ = true;
                isDepthOfField_ = true;
                isSettingsChanged = true;
            }

            if (isSettingsChanged)
            {
                app::camera::CameraManager::Get().SetMotionBlurEnabled(isMotionBlur_);
                app::camera::CameraManager::Get().SetDepthOfFieldEnabled(isDepthOfField_);
            }

            auto* mbOn  = GetUI<app::ui::UIIcon>(Hash32("Word_MotionBlur_ON"));
            auto* mbOff = GetUI<app::ui::UIIcon>(Hash32("Word_MotionBlur_OFF"));
            if (mbOn)  mbOn->isDraw  = isMotionBlur_;
            if (mbOff) mbOff->isDraw = !isMotionBlur_;

            auto* dofOn  = GetUI<app::ui::UIIcon>(Hash32("Word_DepthOfField_ON"));
            auto* dofOff = GetUI<app::ui::UIIcon>(Hash32("Word_DepthOfField_OFF"));
            if (dofOn)  dofOn->isDraw  = isDepthOfField_;
            if (dofOff) dofOff->isDraw = !isDepthOfField_;
        }
    }
}
