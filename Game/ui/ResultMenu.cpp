#include "stdafx.h"
#include "ResultMenu.h"   
#include "ResultSubMenu.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"

namespace app {
    namespace ui {
        ResultMenu::ResultMenu() {}
        ResultMenu::~ResultMenu() {}

        void ResultMenu::InitializeLogic() {
            // ==========================================
            // 各UIパーツを取得して変数に保存
            // ==========================================
            scoreBoard_ = GetUI<UIIcon>(Hash32("ScoreBoard"));
            enemyIcon1_ = GetUI<UIIcon>(Hash32("EnemyIcon_1"));
            enemyIcon2_ = GetUI<UIIcon>(Hash32("EnemyIcon_2"));
            enemyIcon3_ = GetUI<UIIcon>(Hash32("EnemyIcon_3"));
            levelDigit_ = GetUI<UIDigit>(Hash32("LevelDigit"));
            enemyDigit1_ = GetUI<UIDigit>(Hash32("EnemyDigit_1"));
            enemyDigit2_ = GetUI<UIDigit>(Hash32("EnemyDigit_2"));
            enemyDigit3_ = GetUI<UIDigit>(Hash32("EnemyDigit_3"));
            scoreDigit_ = GetUI<UIDigit>(Hash32("ScoreDigit"));
            rankS_ = GetUI<UIIcon>(Hash32("Rank_S"));
            rankSFog_ = GetUI<UIIcon>(Hash32("Rank_S_Fog"));
            rankA_ = GetUI<UIIcon>(Hash32("Rank_A"));
            rankB_ = GetUI<UIIcon>(Hash32("Rank_B"));
            skipIcon_ = GetUI<UIIcon>(Hash32("Skip"));
            nextIcon_ = GetUI<UIIcon>(Hash32("Next"));

            // ==========================================
            // 最初は全て非表示にしておく
            // ==========================================
            if (scoreBoard_)  scoreBoard_->isDraw = false;
            if (enemyIcon1_) enemyIcon1_->isDraw = false;
            if (enemyIcon2_) enemyIcon2_->isDraw = false;
            if (enemyIcon3_) enemyIcon3_->isDraw = false;
            if (levelDigit_)  levelDigit_->isDraw = false;
            if (enemyDigit1_) enemyDigit1_->isDraw = false;
            if (enemyDigit2_) enemyDigit2_->isDraw = false;
            if (enemyDigit3_) enemyDigit3_->isDraw = false;
            if (scoreDigit_)  scoreDigit_->isDraw = false;
            if (rankS_)    rankS_->transform.localScale = Vector3::Zero;
            if (rankSFog_) rankSFog_->transform.localScale = Vector3::Zero;
            if (rankA_)    rankA_->transform.localScale = Vector3::Zero;
            if (rankB_)    rankB_->transform.localScale = Vector3::Zero;

            auto attachStamp = [&](UIDigit* digit) {
                if (!digit) return;
                app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(digit, Hash32("DigitStamp"));
                digit->transform.localScale = Vector3::Zero;
                };
            attachStamp(levelDigit_);
            attachStamp(enemyDigit1_);
            attachStamp(enemyDigit2_);
            attachStamp(enemyDigit3_);
            attachStamp(scoreDigit_);

            if (levelDigit_)  initialDigitPositions_[0] = levelDigit_->transform.localPosition;
            if (enemyDigit1_) initialDigitPositions_[1] = enemyDigit1_->transform.localPosition;
            if (enemyDigit2_) initialDigitPositions_[2] = enemyDigit2_->transform.localPosition;
            if (enemyDigit3_) initialDigitPositions_[3] = enemyDigit3_->transform.localPosition;
            if (scoreDigit_)  initialDigitPositions_[4] = scoreDigit_->transform.localPosition;

            if (skipIcon_) skipIcon_->isDraw = true;
            if (nextIcon_) nextIcon_->isDraw = false;

            // ==========================================
            // アニメーションをアタッチ
            // ==========================================
            if (enemyIcon1_) app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(enemyIcon1_, Hash32("EnemyIcon1Slide"));
            if (enemyIcon2_) app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(enemyIcon2_, Hash32("EnemyIcon2Slide"));
            if (enemyIcon3_) app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(enemyIcon3_, Hash32("EnemyIcon3Slide"));
            if (scoreBoard_) app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(scoreBoard_, Hash32("ScoreBoardSlideIn"));
            if (rankS_) app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(rankS_, Hash32("RankStamp"));
            if (rankA_) app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(rankA_, Hash32("RankStamp"));
            if (rankB_) app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(rankB_, Hash32("RankStamp"));

            // ==========================================
            // 最初の状態をセット
            // ==========================================
            currentState_ = SequenceState::ShowResult;
            stateTimer_ = 2.0f;
        }

        void ResultMenu::Update() {
            MenuBase::Update();

            // ==========================================
            // 演出スキップ処理
            // ==========================================
            if (currentState_ >= SequenceState::ShowResult && currentState_ <= SequenceState::ShowDigits) {
                if (g_pad[0]->IsTrigger(enButtonA)) {
                    if (scoreBoard_)  scoreBoard_->isDraw = true;
                    if (enemyIcon1_) enemyIcon1_->isDraw = true;
                    if (enemyIcon2_) enemyIcon2_->isDraw = true;
                    if (enemyIcon3_) enemyIcon3_->isDraw = true;

                    UIDigit* digits[] = { levelDigit_, enemyDigit1_, enemyDigit2_, enemyDigit3_, scoreDigit_ };
                    for (int i = 0; i < 5; ++i) {
                        if (digits[i]) {
                            digits[i]->isDraw = true;
                            digits[i]->transform.localScale = Vector3::One;
                            digits[i]->transform.localPosition = initialDigitPositions_[i];
                            digits[i]->transform.UpdateTransform(); 
                        }
                    }

                    if (rankA_) rankA_->transform.localScale = Vector3::Zero;
                    if (rankB_) rankB_->transform.localScale = Vector3::Zero;
                    if (rankS_) {
                        rankS_->transform.localScale = Vector3::One;
                        rankS_->transform.localRotation = Quaternion::Identity;
                    }
                    if (rankSFog_) {
                        rankSFog_->transform.localScale = Vector3::One;
                    }
                    debugCurrentRank_ = 0;
                    sFogTimer_ = 0.0f;

                    if (skipIcon_) skipIcon_->isDraw = false;
                    if (nextIcon_) nextIcon_->isDraw = true;

                    currentState_ = SequenceState::Finished;
                    stateTimer_ = 0.0f;
                    return;
                }
            }

            // ==========================================
            // 状態（ステート）ごとの処理
            // ==========================================
            switch (currentState_) {

            case SequenceState::ShowResult:
            {
                stateTimer_ -= g_gameTime->GetFrameDeltaTime();
                if (stateTimer_ <= 0.0f) {
                    currentState_ = SequenceState::SlideInScoreBoardAndResult;

                    if (scoreBoard_) {
                        scoreBoard_->isDraw = true;
                        auto* anim = scoreBoard_->FindAnimation(Hash32("ScoreBoardSlideIn"));
                        if (anim) anim->Play();
                    }
                    if (enemyIcon1_) {
                        enemyIcon1_->isDraw = true;
                        auto* anim = enemyIcon1_->FindAnimation(Hash32("EnemyIcon1Slide"));
                        if (anim) anim->Play();
                    }
                    if (enemyIcon2_) {
                        enemyIcon2_->isDraw = true;
                        auto* anim = enemyIcon2_->FindAnimation(Hash32("EnemyIcon2Slide"));
                        if (anim) anim->Play();
                    }
                    if (enemyIcon3_) {
                        enemyIcon3_->isDraw = true;
                        auto* anim = enemyIcon3_->FindAnimation(Hash32("EnemyIcon3Slide"));
                        if (anim) anim->Play();
                    }
                    stateTimer_ = 1.5f;
                }
                break;
            }

            case SequenceState::SlideInScoreBoardAndResult:
            {
                stateTimer_ -= g_gameTime->GetFrameDeltaTime();
                if (stateTimer_ <= 0.0f) {
                    currentState_ = SequenceState::ShowDigits;
                    stateTimer_ = 0.5f;
                    currentDigitStep_ = 0;
                }
                break;
            }

            case SequenceState::ShowDigits:
            {
                UIDigit* currentDigit = nullptr;
                if (currentDigitStep_ == 0)currentDigit = levelDigit_;
                else if (currentDigitStep_ == 1)currentDigit = enemyDigit1_;
                else if (currentDigitStep_ == 2)currentDigit = enemyDigit2_;
                else if (currentDigitStep_ == 3)currentDigit = enemyDigit3_;
                else if (currentDigitStep_ == 4)currentDigit = scoreDigit_;

                if (currentDigit) {
                    if (!currentDigit->isDraw) {
                        currentDigit->isDraw = true;
                        auto* anim = currentDigit->FindAnimation(Hash32("DigitStamp"));
                        if (anim) anim->Play();

                        stateTimer_ = 0.0f;
                        shakeTimer_ = 0.0f;
                    }

                    auto* anim = currentDigit->FindAnimation(Hash32("DigitStamp"));
                    if (anim && !anim->IsPlay() && shakeTimer_ <= 0.0f && stateTimer_ == 0.0f) {
                        shakeTimer_ = 0.15f;
                        shakingDigitIndex_ = currentDigitStep_;
                        stateTimer_ = 0.15;
                    }
                }

                if (stateTimer_ > 0.0f) {
                    stateTimer_ -= g_gameTime->GetFrameDeltaTime();
                    if (stateTimer_ <= 0.0f) {
                        // 最後の数字まで出し終わった
                        if (currentDigitStep_ == 4) {
                            currentState_ = SequenceState::ShowRank;

                            stateTimer_ = 1.0f;
                        }
                        else {
                            currentDigitStep_++;
                            stateTimer_ = 0.0f;
                        }
                    }
                }
                break;
            }

            case SequenceState::ShowRank:
            {
                stateTimer_ -= g_gameTime->GetFrameDeltaTime();

                if (currentDigitStep_ == 4 && stateTimer_ <= 0.0f) {
                    if (rankA_) rankA_->transform.localScale = Vector3::Zero;
                    if (rankB_) rankB_->transform.localScale = Vector3::Zero;
                    if (rankSFog_) rankSFog_->transform.localScale = Vector3::Zero;
                    if (rankS_) {
                        rankS_->transform.localScale = Vector3::One;
                        rankS_->transform.localRotation = Quaternion::Identity;
                        auto* anim = rankS_->FindAnimation(Hash32("RankStamp"));
                        if (anim) anim->Play();
                    }
                    debugCurrentRank_ = 0;
                    sFogTimer_ = 0.0f;

                    stateTimer_ = 1.5f;
                    currentDigitStep_ = 5; 
                    break;
                }

                // ランク表示中の霧
                if (currentDigitStep_ == 5 && debugCurrentRank_ == 0 && rankSFog_ && rankS_) {
                    sFogTimer_ += g_gameTime->GetFrameDeltaTime();
                    float fogScale = 1.0f + 0.1f * sinf(sFogTimer_ * 5.0f);
                    rankSFog_->transform.localScale = rankS_->transform.localScale * fogScale;
                    rankSFog_->color.w = 0.6f + 0.4f * sinf(sFogTimer_ * 5.0f);
                }

                if (currentDigitStep_ == 5 && stateTimer_ <= 0.0f) {
                    if (skipIcon_) skipIcon_->isDraw = false;
                    if (nextIcon_) nextIcon_->isDraw = true;

                    currentState_ = SequenceState::Finished;
                }
                break;
            }

            case SequenceState::Finished:
            {
                if (nextIcon_ && nextIcon_->isDraw && g_pad[0]->IsTrigger(enButtonA)) {
                    currentState_ = SequenceState::OpenMenu;
                    subMenuLayout_ = std::make_unique<app::ui::Layout>();
                    subMenuLayout_->Initialize<app::ui::ResultSubMenu>("Assets/ui/layout/ResultMenuLayout.json");

                    auto* subMenu = dynamic_cast<app::ui::ResultSubMenu*>(subMenuLayout_->GetMenu());
                    if (subMenu) {
                        subMenu->OnOpen();
                    }
                }

                // デバッグ用:他ランクの処理】
                if (g_pad[0]->IsTrigger(enButtonB)) {
                    if (rankS_) rankS_->transform.localScale = Vector3::Zero;
                    if (rankSFog_) rankSFog_->transform.localScale = Vector3::Zero;
                    if (rankB_) rankB_->transform.localScale = Vector3::Zero;

                    if (rankA_) {
                        rankA_->transform.localScale = Vector3::One;
                        rankA_->transform.localRotation = Quaternion::Identity;
                        auto* anim = rankA_->FindAnimation(Hash32("RankStamp"));
                        if (anim) anim->Play();
                    }
                    debugCurrentRank_ = 1;
                }

                if (g_pad[0]->IsTrigger(enButtonX)) {
                    if (rankS_) rankS_->transform.localScale = Vector3::Zero;
                    if (rankSFog_) rankSFog_->transform.localScale = Vector3::Zero;
                    if (rankA_) rankA_->transform.localScale = Vector3::Zero;

                    if (rankB_) {
                        rankB_->transform.localScale = Vector3::One;
                        rankB_->transform.localRotation.SetRotationDeg(Vector3(0.0f, 0.0f, 1.0f), 0.0f);

                        auto* anim = rankB_->FindAnimation(Hash32("RankStamp"));
                        if (anim) anim->Play();
                    }
                    debugCurrentRank_ = 2;
                    bRankTimer_ = 0.0f;
                    isBRankTilted_ = false;
                }

                if (debugCurrentRank_ == 0 && rankSFog_ && rankS_) {
                    sFogTimer_ += g_gameTime->GetFrameDeltaTime();
                    float fogScale = 1.0f + 0.1f * sinf(sFogTimer_ * 5.0f);
                    rankSFog_->transform.localScale = rankS_->transform.localScale * fogScale;
                    rankSFog_->color.w = 0.6f + 0.4f * sinf(sFogTimer_ * 5.0f);
                }

                if (debugCurrentRank_ == 2 && rankB_ && !isBRankTilted_) {
                    bRankTimer_ += g_gameTime->GetFrameDeltaTime();
                    if (bRankTimer_ >= 1.5f) {
                        float t = (bRankTimer_ - 1.5f) / 0.2f;
                        Vector3 startPos = Vector3(-470.0f, -360.0f, 0.0f);
                        Vector3 targetPos = Vector3(-470.0f, -370.0f, 0.0f);

                        if (t < 1.0f) {
                            float currentAngle = -7.5f * t;
                            rankB_->transform.localRotation.SetRotationDeg(Vector3(0.0f, 0.0f, 1.0f), currentAngle);

                            Vector3 currentPos;
                            currentPos.x = startPos.x + (targetPos.x - startPos.x) * t;
                            currentPos.y = startPos.y + (targetPos.y - startPos.y) * t;
                            currentPos.z = startPos.z;
                            rankB_->transform.localPosition = currentPos;
                        }
                        else {
                            rankB_->transform.localRotation.SetRotationDeg(Vector3(0.0f, 0.0f, 1.0f), -7.5f);
                            isBRankTilted_ = true;
                        }
                        rankB_->transform.UpdateTransform();
                    }
                }
                break;
            }

            case SequenceState::OpenMenu:
            {
                if (subMenuLayout_) {
                    subMenuLayout_->Update();
                }
                break;
            }
            }

            // ==========================================
            // 数字のシェイク処理
            // ==========================================
            if (shakeTimer_ > 0.0f && shakingDigitIndex_ >= 0 && shakingDigitIndex_ <= 4) {
                shakeTimer_ -= g_gameTime->GetFrameDeltaTime();

                UIDigit* targetDigit = nullptr;
                if (shakingDigitIndex_ == 0)targetDigit = levelDigit_;
                else if (shakingDigitIndex_ == 1)targetDigit = enemyDigit1_;
                else if (shakingDigitIndex_ == 2)targetDigit = enemyDigit2_;
                else if (shakingDigitIndex_ == 3)targetDigit = enemyDigit3_;
                else if (shakingDigitIndex_ == 4)targetDigit = scoreDigit_;

                if (targetDigit) {
                    if (shakeTimer_ > 0.0f) {
                        float offsetX = ((rand() % 100) / 100.0f - 0.5f) * 12.0f;
                        float offsetY = ((rand() % 100) / 100.0f - 0.5f) * 12.0f;

                        targetDigit->transform.localPosition = initialDigitPositions_[shakingDigitIndex_] + Vector3(offsetX, offsetY, 0.0f);
                    }
                    else {
                        targetDigit->transform.localPosition = initialDigitPositions_[shakingDigitIndex_];
                    }
                    targetDigit->transform.UpdateTransform();
                }
            }
        }

        void ResultMenu::Render(RenderContext& rc) {
            MenuBase::Render(rc);
            if (currentState_ == SequenceState::OpenMenu && subMenuLayout_) {
                subMenuLayout_->Render(rc);
            }
        }

        void ResultMenu::OnOpen() {}
        void ResultMenu::OnClose() {}

        bool ResultMenu::IsReturnTitleDecided() const {
            if (subMenuLayout_) {
                auto* subMenu = dynamic_cast<ResultSubMenu*>(subMenuLayout_->GetMenu());
                if (subMenu) return subMenu->IsReturnTitleDecided();
            }
            return false;
        }

        bool ResultMenu::IsRetryDecided() const {
            if (subMenuLayout_) {
                auto* subMenu = dynamic_cast<ResultSubMenu*>(subMenuLayout_->GetMenu());
                if (subMenu) return subMenu->IsRetryDecided();
            }
            return false;
        }

        bool ResultMenu::IsExitDecided() const {
            if (subMenuLayout_) {
                auto* subMenu = dynamic_cast<ResultSubMenu*>(subMenuLayout_->GetMenu());
                if (subMenu) return subMenu->IsExitDecided();
            }
            return false;
        }
    }
}