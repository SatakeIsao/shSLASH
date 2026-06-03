#include "stdafx.h"
#include "CameraOptionMenu.h"
#include "sound/SoundManager.h"
#include "core/ParameterManager.h"
#include "camera/CameraManager.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"
// #include "core/ParameterManager.h" // 後でカメラパラメーターを実際に反映する時に使います

namespace app
{
    namespace ui
    {
        CameraOptionMenu::CameraOptionMenu()
        {
            app::core::ParameterManager::Get().LoadParameter<app::core::CameraOptionMenuParameter>("Assets/master/CameraOptionMenuParameter.json", [](const nlohmann::json& j, app::core::CameraOptionMenuParameter& p)
                {
                    p.cursolPositionX[0] = j["cursolPositionXA"];
                    p.cursolPositionX[1] = j["cursolPositionXB"];
                    p.cursolPositionX[2] = j["cursolPositionXC"];

                    p.cursolPositionY[0] = j["cursolPositionYA"];
                    p.cursolPositionY[1] = j["cursolPositionYB"];
                    p.cursolPositionY[2] = j["cursolPositionYC"];

                    p.highlightPositionX[0] = j["highlightPositionXA"];
                    p.highlightPositionX[1] = j["highlightPositionXB"];
                    p.highlightPositionX[2] = j["highlightPositionXC"];

                    p.highlightPositionY[0] = j["highlightPositionYA"];
                    p.highlightPositionY[1] = j["highlightPositionYB"];
                    p.highlightPositionY[2] = j["highlightPositionYC"];

                    char scaleStr[] = "barScaleXA"; 
                    for (uint32_t i = 0; i < 11; ++i) {
                        scaleStr[9] = 'A' + i; 
                        p.barScaleX[i] = j[scaleStr];
                    }
                });
        }

        CameraOptionMenu::~CameraOptionMenu()
        {
            app::core::ParameterManager::Get().UnloadParameter<app::core::CameraOptionMenuParameter>();
        }

        void CameraOptionMenu::InitializeLogic()
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

            auto* parameter = app::core::ParameterManager::Get().GetParameter<app::core::CameraOptionMenuParameter>();
            if (parameter)
            {
                auto* cursor = GetUI<app::ui::UIIcon>(Hash32("Cursor"));
                if (cursor)
                {
                    cursor->transform.localPosition.x = parameter->cursolPositionX[cursorIndex_];
                    cursor->transform.localPosition.y = parameter->cursolPositionY[cursorIndex_];
                    cursor->transform.UpdateTransform();
                }

                auto* highlight = GetUI<app::ui::UIIcon>(Hash32("Highlight"));
                if (highlight)
                {
                    highlight->transform.localPosition.x = parameter->highlightPositionX[cursorIndex_];
                    highlight->transform.localPosition.y = parameter->highlightPositionY[cursorIndex_];
                    highlight->transform.UpdateTransform();
                }


            }

            auto* speedBar = GetUI<app::ui::UIIcon>(Hash32("SpeedBar"));
            if (speedBar) {
                speedBar->SetPivot(Vector2(0.0f, 0.5f));
            }

            isXReverse_ = app::camera::CameraManager::Get().IsReverseX();
            isYReverse_ = app::camera::CameraManager::Get().IsReverseY();
            // TODO: 初期化時に ParameterManager 等から現在のカメラ設定を読み込んで isXReverse_, isYReverse_ にセットする

            auto* highlight = GetUI<app::ui::UIIcon>(Hash32("Highlight"));
            if (highlight)
            {
                app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(highlight, Hash32("FadeIn"));
            }
        }

        void CameraOptionMenu::OnOpen()
        {
            SetDraw(true);
        }

        void CameraOptionMenu::OnClose()
        {
            SetDraw(false);
        }

        void CameraOptionMenu::SetDraw(bool isDraw)
        {
            auto* canvas = GetCanvas();
            if (canvas) {
                canvas->isDraw = isDraw;
            }

            auto* highlight = GetUI<app::ui::UIIcon>(Hash32("Highlight"));
            if (isDraw) {
                if (highlight) {
                    auto* anim = highlight->FindAnimation(Hash32("FadeIn"));
                    if (anim) anim->Play();
                }
            }
            else {
                if (highlight) highlight->StopSpriteAnimation();
            }
        }

        void CameraOptionMenu::Update()
        {
            auto* canvas = GetCanvas();
            if (!canvas || !canvas->isDraw) return;

            MenuBase::Update();

            // ==========================================
            // カーソルの上下移動処理
            // ==========================================
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

            // ==========================================
            // カーソルの座標更新
            // ==========================================
            auto* parameter = app::core::ParameterManager::Get().GetParameter<app::core::CameraOptionMenuParameter>();
            if (parameter)
            {
                auto* cursor = GetUI<app::ui::UIIcon>(Hash32("Cursor"));
                if (cursor)
                {
                    cursor->transform.localPosition.x = parameter->cursolPositionX[cursorIndex_];
                    cursor->transform.localPosition.y = parameter->cursolPositionY[cursorIndex_];
                    cursor->transform.UpdateTransform();
                }

                auto* highlight = GetUI<app::ui::UIIcon>(Hash32("Highlight"));
                if (highlight)
                {
                    highlight->transform.localPosition.x = parameter->highlightPositionX[cursorIndex_];
                    highlight->transform.localPosition.y = parameter->highlightPositionY[cursorIndex_];
                    highlight->transform.UpdateTransform();
                }
            }

            bool isSettingsChanged = false; 
            float currentSensitivity = app::camera::CameraManager::Get().GetSensitivity();

            // ==========================================
            // 左右ボタンによる ノーマル / リバース の切り替え処理
            // ==========================================

            if (cursorIndex_ == 0 || cursorIndex_ == 1)
            {
                if (g_pad[0]->IsTrigger(enButtonLeft) || g_pad[0]->IsTrigger(enButtonRight))
                {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonMove));
                    if (cursorIndex_ == 0) isXReverse_ = !isXReverse_;
                    if (cursorIndex_ == 1) isYReverse_ = !isYReverse_;
                    isSettingsChanged = true;
                }
            }
            else if (cursorIndex_ == 2)
            {
                // 感度の調整処理
                if (g_pad[0]->IsTrigger(enButtonRight))
                {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonMove));
                    currentSensitivity += 0.1f;
                    if (currentSensitivity > 1.0f) currentSensitivity = 1.0f;
                    isSettingsChanged = true;
                }
                else if (g_pad[0]->IsTrigger(enButtonLeft))
                {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonMove));
                    currentSensitivity -= 0.1f;
                    // 完全に0になるとカメラが動かなくなるので最低0.1でストップ
                    if (currentSensitivity < 0.0f) currentSensitivity = 0.0f;
                    isSettingsChanged = true;
                }
            }

            // ==========================================
            // デフォルトに戻す処理
            // ==========================================

            if (g_pad[0]->IsTrigger(enButtonY))
            {
                app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonReset));

                isXReverse_ = false;
                isYReverse_ = false;
                currentSensitivity = 0.5f;
                isSettingsChanged = true;
            }

            if (isSettingsChanged)
            {
                app::camera::CameraManager::Get().SetReverseX(isXReverse_);
                app::camera::CameraManager::Get().SetReverseY(isYReverse_);
                app::camera::CameraManager::Get().SetSensitivity(currentSensitivity);
            }

            // --- X軸のUI更新 ---
            auto* normalX = GetUI<app::ui::UIIcon>(Hash32("Word_Normal_UD"));
            auto* reverseX = GetUI<app::ui::UIIcon>(Hash32("Word_Reverse_UD"));
            if (normalX)  normalX->isDraw = !isXReverse_; // ノーマル(false)の時だけ表示
            if (reverseX) reverseX->isDraw = isXReverse_;  // リバース(true)の時だけ表示

            // --- Y軸のUI更新 ---
            auto* normalY = GetUI<app::ui::UIIcon>(Hash32("Word_Normal_LR"));
            auto* reverseY = GetUI<app::ui::UIIcon>(Hash32("Word_Reverse_LR"));
            if (normalY)  normalY->isDraw = !isYReverse_;
            if (reverseY) reverseY->isDraw = isYReverse_;

            if (parameter)
            {
                // 0.0〜1.0 の値を 0〜10 のインデックスに変換
                int sensIndex = static_cast<int>(std::round(currentSensitivity * 10.0f));
                if (sensIndex < 0) sensIndex = 0;
                if (sensIndex > 10) sensIndex = 10;

                auto* speedBar = GetUI<app::ui::UIIcon>(Hash32("SpeedBar"));
                if (speedBar) {
                    // JSONから読み込んだ A〜K のスケール値をそのまま適用
                    speedBar->transform.localScale.x = parameter->barScaleX[sensIndex];
                }
            }
        }
    }
}