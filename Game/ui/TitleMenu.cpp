#include "stdafx.h"
#include "TitleMenu.h"
#include "ui/UIAnimationFactory.h" 
#include "ui/UIAnimation.h"        

namespace app {
    namespace ui {

        void TitleMenu::InitializeLogic() {
            pressA_ = GetUI<app::ui::UIIcon>(Hash32("GuideA"));
            titleLeft_ = GetUI<app::ui::UIIcon>(Hash32("TitleLeft"));
            titleRight_ = GetUI<app::ui::UIIcon>(Hash32("TitleRight"));
            slashedEffect_ = GetUI<app::ui::UIIcon>(Hash32("SlashedEffect"));

            // ワイプパラメータを初期化
            wipeParam_.wipeDir.Set(-0.7071f, 0.7071f);
            wipeParam_.wipeDir.Normalize();
            wipeParam_.wipeSize = 99999.0; // 右上から開始

            // SlashEffectをカスタムシェーダーで初期化
            SpriteInitData initData;
            initData.m_ddsFilePath[0] = "Assets/ui/title/slashEffect.DDS";
            initData.m_fxFilePath = "Assets/Shader/wipe.fx"; // ワイプシェーダー
            initData.m_width = 288.6f;
            initData.m_height = 441.0f;
            initData.m_expandConstantBuffer = &wipeParam_;
            initData.m_expandConstantBufferSize = sizeof(wipeParam_);
            initData.m_colorBufferFormat[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            initData.m_alphaBlendMode = AlphaBlendMode_Trans;
            slashSpriteRender_.Init(initData);
            slashSpriteRender_.SetPosition(Vector3(0.0f, 108.0f, 0.0f));

            if (pressA_) {
                app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(pressA_, Hash32("FadeIn"));
                auto* anim = pressA_->FindAnimation(Hash32("FadeIn"));
                if (anim) anim->Play();
            }
        }

        void TitleMenu::Update() {
            float dt = g_gameTime->GetFrameDeltaTime();
            if (dt <= 0.0f) dt = 1.0f / 60.0f;

            // ワイプの更新
            if (isWiping_ && isSlashVisible_) {
                wipeTimer_ += dt;

                float t = wipeTimer_ / wipeDuration_;
                if (t > 1.0f) t = 1.0f;
                // 指数関数的加速
                t = t * t * t;

                wipeParam_.wipeSize = -600.9f + (1000.0f - (-600.9f)) * t;

                if (wipeTimer_ >= wipeDuration_) {
                    wipeParam_.wipeSize = 1000.0f;
                    isWiping_ = false;
                    wipeTimer_ = 0.0f;

                    // ワイプ完了後にロゴ分割アニメーション開始
                    if (titleLeft_) {
                        app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleLeft_, Hash32("TitleLogoSplitLeft_1"));
                        auto* anim = titleLeft_->FindAnimation(Hash32("TitleLogoSplitLeft_1"));
                        if (anim) anim->Play();
                    }
                    if (titleRight_) {
                        app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleRight_, Hash32("TitleLogoSplitRight_1"));
                        auto* anim = titleRight_->FindAnimation(Hash32("TitleLogoSplitRight_1"));
                        if (anim) anim->Play();
                    }
                    animState_ = AnimState::Opening_Step1;
                    animTimer_ = 0.06f;
                }
            }
            slashSpriteRender_.Update();

            // Opening_Step1：分割アニメーション待機 → 衝撃シェイク開始
            if (animState_ == AnimState::Opening_Step1) {
                animTimer_ -= dt;
                if (animTimer_ <= 0.0f) {
                    if (titleLeft_) {
                        //titleLeft_->RemoveAnimation(Hash32("TitleLogoSplitLeft_1"));
                        app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleLeft_, Hash32("TitleLogoShakeLeft_1"));
                        auto* anim = titleLeft_->FindAnimation(Hash32("TitleLogoShakeLeft_1"));
                        if (anim) anim->Play();
                    }
                    if (titleRight_) {
                        //titleRight_->RemoveAnimation(Hash32("TitleLogoSplitRight_1"));
                        app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleRight_, Hash32("TitleLogoShakeRight_1"));
                        auto* anim = titleRight_->FindAnimation(Hash32("TitleLogoShakeRight_1"));
                        if (anim) anim->Play();
                    }
                    animState_ = AnimState::ShakeStep1;
                    animTimer_ = 0.01f;
                }
            }

            // ShakeStep1：跳ね上がり待機 → 元に戻る
            else if (animState_ == AnimState::ShakeStep1) {
                animTimer_ -= dt;
                if (animTimer_ <= 0.0f) {
                    if (titleLeft_) {
                        titleLeft_->RemoveAnimation(Hash32("TitleLogoShakeLeft_1"));
                        app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleLeft_, Hash32("TitleLogoShakeLeft_2"));
                        auto* anim = titleLeft_->FindAnimation(Hash32("TitleLogoShakeLeft_2"));
                        if (anim) anim->Play();
                    }
                    if (titleRight_) {
                        titleRight_->RemoveAnimation(Hash32("TitleLogoShakeRight_1"));
                        app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleRight_, Hash32("TitleLogoShakeRight_2"));
                        auto* anim = titleRight_->FindAnimation(Hash32("TitleLogoShakeRight_2"));
                        if (anim) anim->Play();
                    }
                    animState_ = AnimState::ShakeStep2;
                    animTimer_ = 0.10f;
                }
            }

            // ShakeStep2：戻り待機 → 2回目跳ねへ
            else if (animState_ == AnimState::ShakeStep2) {
                animTimer_ -= dt;
                if (animTimer_ <= 0.0f) {
                    if (titleLeft_) {
                        titleLeft_->RemoveAnimation(Hash32("TitleLogoShakeLeft_2"));
                        app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleLeft_, Hash32("TitleLogoShakeLeft_3"));
                        auto* anim = titleLeft_->FindAnimation(Hash32("TitleLogoShakeLeft_3"));
                        if (anim) anim->Play();
                    }
                    if (titleRight_) {
                        titleRight_->RemoveAnimation(Hash32("TitleLogoShakeRight_2"));
                        app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleRight_, Hash32("TitleLogoShakeRight_3"));
                        auto* anim = titleRight_->FindAnimation(Hash32("TitleLogoShakeRight_3"));
                        if (anim) anim->Play();
                    }
                    animState_ = AnimState::ShakeStep3;
                    animTimer_ = 0.06f;
                }
            }

            // ShakeStep3：2回目跳ね待機 → 元に戻る
            else if (animState_ == AnimState::ShakeStep3) {
                animTimer_ -= dt;
                if (animTimer_ <= 0.0f) {
                    if (titleLeft_) {
                        titleLeft_->RemoveAnimation(Hash32("TitleLogoShakeLeft_3"));
                        app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleLeft_, Hash32("TitleLogoShakeLeft_4"));
                        auto* anim = titleLeft_->FindAnimation(Hash32("TitleLogoShakeLeft_4"));
                        if (anim) anim->Play();
                    }
                    if (titleRight_) {
                        titleRight_->RemoveAnimation(Hash32("TitleLogoShakeRight_3"));
                        app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleRight_, Hash32("TitleLogoShakeRight_4"));
                        auto* anim = titleRight_->FindAnimation(Hash32("TitleLogoShakeRight_4"));
                        if (anim) anim->Play();
                    }
                    animState_ = AnimState::ShakeStep4;
                    animTimer_ = 0.10f;
                }
            }

            // ShakeStep4：2回目戻り待機 → Opening_Step2へ
            else if (animState_ == AnimState::ShakeStep4) {
                animTimer_ -= dt;
                if (animTimer_ <= 0.0f) {
                    if (titleLeft_) {
                        titleLeft_->RemoveAnimation(Hash32("TitleLogoShakeLeft_4"));
                        app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleLeft_, Hash32("TitleLogoSplitLeft_2"));
                        auto* anim = titleLeft_->FindAnimation(Hash32("TitleLogoSplitLeft_2"));
                        if (anim) anim->Play();
                    }
                    if (titleRight_) {
                        titleRight_->RemoveAnimation(Hash32("TitleLogoShakeRight_4"));
                        app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleRight_, Hash32("TitleLogoSplitRight_2"));
                        auto* anim = titleRight_->FindAnimation(Hash32("TitleLogoSplitRight_2"));
                        if (anim) anim->Play();
                    }
                    animState_ = AnimState::Opening_Step2;
                    animTimer_ = 0.4f;
                }
            }

            // Closing：タイトルに戻るとき
            else if (animState_ == AnimState::Closing) {
                animTimer_ -= dt;
                if (animTimer_ <= 0.0f) {
                    animState_ = AnimState::None;
                    if (pressA_) pressA_->isDraw = true;
                }
            }

            MenuBase::Update();
        }

        void TitleMenu::OnOpen() {
            animState_ = AnimState::Closing;
            animTimer_ = 0.4f;

            if (pressA_) pressA_->isDraw = false;
            if (slashedEffect_) slashedEffect_->isDraw = false;

            // スラッシュエフェクトを非表示にリセット
            isSlashVisible_ = false;
            isWiping_ = false;
            wipeParam_.wipeSize = -600.9f;

            if (titleLeft_) {
                titleLeft_->RemoveAnimation(Hash32("TitleLogoSplitLeft_2"));
                titleLeft_->RemoveAnimation(Hash32("TitleLogoReturnLeft"));
                app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleLeft_, Hash32("TitleLogoReturnLeft"));
                auto* anim = titleLeft_->FindAnimation(Hash32("TitleLogoReturnLeft"));
                if (anim) anim->Play();
            }

            if (titleRight_) {
                titleRight_->RemoveAnimation(Hash32("TitleLogoSplitRight_2"));
                titleRight_->RemoveAnimation(Hash32("TitleLogoReturnRight"));
                app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(titleRight_, Hash32("TitleLogoReturnRight"));
                auto* anim = titleRight_->FindAnimation(Hash32("TitleLogoReturnRight"));
                if (anim) anim->Play();
            }
        }

        void TitleMenu::OnClose() {
            animState_ = AnimState::None;

            if (pressA_) pressA_->isDraw = false;
            if (slashedEffect_) slashedEffect_->isDraw = true;

            if (titleLeft_) {
                titleLeft_->RemoveAnimation(Hash32("TitleLogoReturnLeft"));
                titleLeft_->RemoveAnimation(Hash32("TitleLogoSplitLeft_1"));
            }

            if (titleRight_) {
                titleRight_->RemoveAnimation(Hash32("TitleLogoReturnRight"));
                titleRight_->RemoveAnimation(Hash32("TitleLogoSplitRight_1"));
            }

            // スラッシュエフェクトのワイプ開始
            wipeParam_.wipeSize = -600.9f; // 右上にリセット
            isSlashVisible_ = true;
            isWiping_ = true;
        }

        bool TitleMenu::IsPlaying() {
            return animState_ != AnimState::None;
        }
    }
}