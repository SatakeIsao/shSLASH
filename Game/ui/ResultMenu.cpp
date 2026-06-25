#include "stdafx.h"
#include "ResultMenu.h"
#include "ResultSubMenu.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"
#include "GameResultData.h"
#include "sound/SoundManager.h"

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
            levelDigit_ = GetUI<UINumberSprite>(Hash32("LevelDigit"));
            enemyDigit1_ = GetUI<UINumberSprite>(Hash32("EnemyDigit_1"));
            enemyDigit2_ = GetUI<UINumberSprite>(Hash32("EnemyDigit_2"));
            enemyDigit3_ = GetUI<UINumberSprite>(Hash32("EnemyDigit_3"));
            scoreDigit_ = GetUI<UINumberSprite>(Hash32("ScoreDigit"));
            rankMaster_ = GetUI<UIIcon>(Hash32("Rank_S"));
            rankMasterFog_ = GetUI<UIIcon>(Hash32("Rank_S_Fog"));
            rankElite_ = GetUI<UIIcon>(Hash32("Rank_A"));
            rankBeginner_ = GetUI<UIIcon>(Hash32("Rank_B"));
            resultIcon_ = GetUI<UIIcon>(Hash32("Result"));
            skipIcon_ = GetUI<UIIcon>(Hash32("Skip"));
            nextIcon_ = GetUI<UIIcon>(Hash32("Next"));

            const app::GameResultData& rd = app::GameResultData::Get();
            currentRank_ = static_cast<int>(rd.CalcRank());
            if (levelDigit_)  levelDigit_->SetNumber(rd.level);
            if (enemyDigit1_) enemyDigit1_->SetNumber(rd.stoneKillCount);
            if (enemyDigit2_) enemyDigit2_->SetNumber(rd.mushroomKillCount);
            if (enemyDigit3_) enemyDigit3_->SetNumber(0);
            if (scoreDigit_)  scoreDigit_->SetNumber(rd.CalcTotalScore());

            if (resultIcon_)    resultIcon_->isDraw = false;
            if (scoreBoard_)    scoreBoard_->isDraw = false;
            if (enemyIcon1_)    enemyIcon1_->isDraw = false;
            if (enemyIcon2_)    enemyIcon2_->isDraw = false;
            if (enemyIcon3_)    enemyIcon3_->isDraw = false;
            if (levelDigit_)    levelDigit_->isDraw = false;
            if (enemyDigit1_)   enemyDigit1_->isDraw = false;
            if (enemyDigit2_)   enemyDigit2_->isDraw = false;
            if (enemyDigit3_)   enemyDigit3_->isDraw = false;
            if (scoreDigit_)    scoreDigit_->isDraw = false;
            if (rankMaster_)    rankMaster_->transform.localScale = Vector3::Zero;
            if (rankMasterFog_) rankMasterFog_->transform.localScale = Vector3::Zero;
            if (rankElite_)     rankElite_->transform.localScale = Vector3::Zero;
            if (rankBeginner_)  rankBeginner_->transform.localScale = Vector3::Zero;

            auto attachStamp = [&](UINumberSprite* digit) {
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
            if (rankBeginner_) initialBeginnerRankPos_ = rankBeginner_->transform.localPosition;

            if (skipIcon_) skipIcon_->isDraw = true;
            if (nextIcon_) nextIcon_->isDraw = false;

            if (enemyIcon1_) app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(enemyIcon1_, Hash32("EnemyIcon1Slide"));
            if (enemyIcon2_) app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(enemyIcon2_, Hash32("EnemyIcon2Slide"));
            if (enemyIcon3_) app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(enemyIcon3_, Hash32("EnemyIcon3Slide"));
            if (resultIcon_) app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(resultIcon_, Hash32("ResultSlide"));
            if (scoreBoard_) app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(scoreBoard_, Hash32("ScoreBoardSlideIn"));
            if (rankMaster_)   app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(rankMaster_, Hash32("RankStamp"));
            if (rankElite_)    app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(rankElite_, Hash32("RankStamp"));
            if (rankBeginner_) app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(rankBeginner_, Hash32("RankStamp"));

            currentState_ = SequenceState::ShowResult;
            stateTimer_ = 2.0f;
        }

        void ResultMenu::Update() {
            MenuBase::Update();

            if (currentState_ >= SequenceState::ShowResult && currentState_ <= SequenceState::ShowDigits) {
                if (g_pad[0]->IsTrigger(enButtonA)) {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonDecision));
                    if (resultIcon_) {
                        resultIcon_->isDraw = true;
                        resultIcon_->transform.localPosition = Vector3(-475.0f, 350.0f, 0.0f);
                        resultIcon_->transform.UpdateTransform();
                    }
                    if (scoreBoard_) {
                        scoreBoard_->isDraw = true;
                        scoreBoard_->transform.localPosition = Vector3(-475.0f, -50.0f, 0.0f);
                        scoreBoard_->transform.UpdateTransform();
                    }
                    if (enemyIcon1_) {
                        enemyIcon1_->isDraw = true;
                        enemyIcon1_->transform.localPosition = Vector3(-475.0f, 45.0f, 0.0f);
                        enemyIcon1_->transform.UpdateTransform();
                    }
                    if (enemyIcon2_) {
                        enemyIcon2_->isDraw = true;
                        enemyIcon2_->transform.localPosition = Vector3(-475.0f, -20.0f, 0.0f);
                        enemyIcon2_->transform.UpdateTransform();
                    }
                    if (enemyIcon3_) {
                        enemyIcon3_->isDraw = true;
                        enemyIcon3_->transform.localPosition = Vector3(-475.0f, -90.0f, 0.0f);
                        enemyIcon3_->transform.UpdateTransform();
                    }

                    UINumberSprite* digits[] = { levelDigit_, enemyDigit1_, enemyDigit2_, enemyDigit3_, scoreDigit_ };
                    for (int i = 0; i < 5; ++i) {
                        if (digits[i]) {
                            digits[i]->isDraw = true;
                            digits[i]->transform.localScale = Vector3::One;
                            digits[i]->transform.localPosition = initialDigitPositions_[i];
                            digits[i]->transform.UpdateTransform();
                        }
                    }

                    if (rankElite_)     rankElite_->transform.localScale = Vector3::Zero;
                    if (rankBeginner_)  rankBeginner_->transform.localScale = Vector3::Zero;
                    if (rankMasterFog_) rankMasterFog_->transform.localScale = Vector3::Zero;
                    if (currentRank_ == 0 && rankMaster_) {
                        rankMaster_->transform.localScale = Vector3::One;
                        rankMaster_->transform.localRotation = Quaternion::Identity;
                    }
                    else if (currentRank_ == 1 && rankElite_) {
                        rankElite_->transform.localScale = Vector3::One;
                        rankElite_->transform.localRotation = Quaternion::Identity;
                    }
                    else if (currentRank_ == 2 && rankBeginner_) {
                        rankBeginner_->transform.localScale = Vector3::One;
                        rankBeginner_->transform.localRotation.SetRotationDeg(Vector3(0.0f, 0.0f, 1.0f), 0.0f);
                    }
                    masterFogTimer_ = 0.0f;

                    if (skipIcon_) skipIcon_->isDraw = false;
                    if (nextIcon_) nextIcon_->isDraw = true;

                    currentState_ = SequenceState::Finished;
                    stateTimer_ = 0.0f;
                    return;
                }
            }

            switch (currentState_) {

            case SequenceState::ShowResult:
            {
                stateTimer_ -= g_gameTime->GetFrameDeltaTime();
                if (stateTimer_ <= 0.0f) {
                    currentState_ = SequenceState::SlideInScoreBoardAndResult;

                    if (resultIcon_) {
                        resultIcon_->isDraw = true;
                        auto* anim = resultIcon_->FindAnimation(Hash32("ResultSlide"));
                        if (anim) anim->Play();
                    }
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
                UINumberSprite* currentDigit = nullptr;
                if (currentDigitStep_ == 0)      currentDigit = levelDigit_;
                else if (currentDigitStep_ == 1) currentDigit = enemyDigit1_;
                else if (currentDigitStep_ == 2) currentDigit = enemyDigit2_;
                else if (currentDigitStep_ == 3) currentDigit = enemyDigit3_;
                else if (currentDigitStep_ == 4) currentDigit = scoreDigit_;

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
                        app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::TitleBreak));
                        shakeTimer_ = 0.15f;
                        shakingDigitIndex_ = currentDigitStep_;
                        stateTimer_ = 0.15;
                    }
                }

                if (stateTimer_ > 0.0f) {
                    stateTimer_ -= g_gameTime->GetFrameDeltaTime();
                    if (stateTimer_ <= 0.0f) {
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
                    if (rankElite_)     rankElite_->transform.localScale = Vector3::Zero;
                    if (rankBeginner_)  rankBeginner_->transform.localScale = Vector3::Zero;
                    if (rankMasterFog_) rankMasterFog_->transform.localScale = Vector3::Zero;
                    if (currentRank_ == 0 && rankMaster_) {
                        rankMaster_->transform.localScale = Vector3::One;
                        rankMaster_->transform.localRotation = Quaternion::Identity;
                        auto* anim = rankMaster_->FindAnimation(Hash32("RankStamp"));
                        if (anim) anim->Play();
                    }
                    else if (currentRank_ == 1 && rankElite_) {
                        rankElite_->transform.localScale = Vector3::One;
                        rankElite_->transform.localRotation = Quaternion::Identity;
                        auto* anim = rankElite_->FindAnimation(Hash32("RankStamp"));
                        if (anim) anim->Play();
                    }
                    else if (currentRank_ == 2 && rankBeginner_) {
                        rankBeginner_->transform.localScale = Vector3::One;
                        rankBeginner_->transform.localRotation.SetRotationDeg(Vector3(0.0f, 0.0f, 1.0f), 0.0f);
                        auto* anim = rankBeginner_->FindAnimation(Hash32("RankStamp"));
                        if (anim) anim->Play();
                        beginnerRankTimer_ = 0.0f;
                        isBeginnerRankTilted_ = false;
                    }
                    masterFogTimer_ = 0.0f;
                    stateTimer_ = 1.5f;
                    currentDigitStep_ = 5;
                    break;
                }

                if (currentDigitStep_ == 5 && !rankSEPlayed_) {
                    UIIcon* rankIcon = (currentRank_ == 0) ? rankMaster_ : (currentRank_ == 1) ? rankElite_ : rankBeginner_;
                    if (rankIcon) {
                        auto* anim = rankIcon->FindAnimation(Hash32("RankStamp"));
                        if (anim && !anim->IsPlay()) {
                            app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::TitleBreak));
                            rankSEPlayed_ = true;
                        }
                    }
                }

                if (currentDigitStep_ == 5 && currentRank_ == 0 && rankMasterFog_ && rankMaster_) {
                    masterFogTimer_ += g_gameTime->GetFrameDeltaTime();
                    float fogScale = 1.0f + 0.1f * sinf(masterFogTimer_ * 5.0f);
                    rankMasterFog_->transform.localScale = rankMaster_->transform.localScale * fogScale;
                    rankMasterFog_->color.w = 0.6f + 0.4f * sinf(masterFogTimer_ * 5.0f);
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
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonDecision));
                    currentState_ = SequenceState::OpenMenu;
                    subMenuLayout_ = std::make_unique<app::ui::Layout>();
                    subMenuLayout_->Initialize<app::ui::ResultSubMenu>("Assets/ui/layout/ResultMenuLayout.json");

                    auto* subMenu = dynamic_cast<app::ui::ResultSubMenu*>(subMenuLayout_->GetMenu());
                    if (subMenu) {
                        subMenu->OnOpen();
                    }
                }

                if (currentRank_ == 0 && rankMasterFog_ && rankMaster_) {
                    masterFogTimer_ += g_gameTime->GetFrameDeltaTime();
                    float fogScale = 1.0f + 0.1f * sinf(masterFogTimer_ * 5.0f);
                    rankMasterFog_->transform.localScale = rankMaster_->transform.localScale * fogScale;
                    rankMasterFog_->color.w = 0.6f + 0.4f * sinf(masterFogTimer_ * 5.0f);
                }

                if (currentRank_ == 2 && rankBeginner_ && !isBeginnerRankTilted_) {
                    beginnerRankTimer_ += g_gameTime->GetFrameDeltaTime();
                    if (beginnerRankTimer_ >= 1.5f) {
                        float t = (beginnerRankTimer_ - 1.5f) / 0.2f;
                        Vector3 startPos = Vector3(-470.0f, -360.0f, 0.0f);
                        Vector3 targetPos = Vector3(-470.0f, -370.0f, 0.0f);
                        if (t < 1.0f) {
                            float currentAngle = -7.5f * t;
                            rankBeginner_->transform.localRotation.SetRotationDeg(Vector3(0.0f, 0.0f, 1.0f), currentAngle);
                            Vector3 currentPos;
                            currentPos.x = startPos.x + (targetPos.x - startPos.x) * t;
                            currentPos.y = startPos.y + (targetPos.y - startPos.y) * t;
                            currentPos.z = startPos.z;
                            rankBeginner_->transform.localPosition = currentPos;
                        }
                        else {
                            rankBeginner_->transform.localRotation.SetRotationDeg(Vector3(0.0f, 0.0f, 1.0f), -7.5f);
                            isBeginnerRankTilted_ = true;
                        }
                        rankBeginner_->transform.UpdateTransform();
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

            if (shakeTimer_ > 0.0f && shakingDigitIndex_ >= 0 && shakingDigitIndex_ <= 4) {
                shakeTimer_ -= g_gameTime->GetFrameDeltaTime();

                UINumberSprite* targetDigit = nullptr;
                if (shakingDigitIndex_ == 0)      targetDigit = levelDigit_;
                else if (shakingDigitIndex_ == 1) targetDigit = enemyDigit1_;
                else if (shakingDigitIndex_ == 2) targetDigit = enemyDigit2_;
                else if (shakingDigitIndex_ == 3) targetDigit = enemyDigit3_;
                else if (shakingDigitIndex_ == 4) targetDigit = scoreDigit_;

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

        void ResultMenu::Reset(int newRank) {
            currentRank_ = newRank;

            // すべての UI を非表示・初期スケールに戻す
            if (resultIcon_)    resultIcon_->isDraw = false;
            if (scoreBoard_)    scoreBoard_->isDraw = false;
            if (enemyIcon1_)    enemyIcon1_->isDraw = false;
            if (enemyIcon2_)    enemyIcon2_->isDraw = false;
            if (enemyIcon3_)    enemyIcon3_->isDraw = false;
            if (levelDigit_)    levelDigit_->isDraw = false;
            if (enemyDigit1_)   enemyDigit1_->isDraw = false;
            if (enemyDigit2_)   enemyDigit2_->isDraw = false;
            if (enemyDigit3_)   enemyDigit3_->isDraw = false;
            if (scoreDigit_)    scoreDigit_->isDraw = false;

            if (rankMaster_) {
                rankMaster_->transform.localScale = Vector3::Zero;
                rankMaster_->transform.UpdateTransform();
            }
            if (rankMasterFog_) {
                rankMasterFog_->transform.localScale = Vector3::Zero;
                rankMasterFog_->transform.UpdateTransform();
            }
            if (rankElite_) {
                rankElite_->transform.localScale = Vector3::Zero;
                rankElite_->transform.UpdateTransform();
            }
            if (rankBeginner_) {
                rankBeginner_->transform.localScale = Vector3::Zero;
                rankBeginner_->transform.localRotation.SetRotationDeg(Vector3(0.0f, 0.0f, 1.0f), 0.0f);
                rankBeginner_->transform.localPosition = initialBeginnerRankPos_;
                rankBeginner_->transform.UpdateTransform();
            }

            UINumberSprite* digits[] = { levelDigit_, enemyDigit1_, enemyDigit2_, enemyDigit3_, scoreDigit_ };
            for (int i = 0; i < 5; ++i) {
                if (digits[i]) {
                    digits[i]->transform.localScale = Vector3::Zero;
                    digits[i]->transform.localPosition = initialDigitPositions_[i];
                    digits[i]->transform.UpdateTransform();
                }
            }

            if (skipIcon_) skipIcon_->isDraw = true;
            if (nextIcon_) nextIcon_->isDraw = false;

            subMenuLayout_.reset();

            currentState_         = SequenceState::ShowResult;
            stateTimer_           = 2.0f;
            currentDigitStep_     = 0;
            masterFogTimer_       = 0.0f;
            beginnerRankTimer_    = 0.0f;
            isBeginnerRankTilted_ = false;
            shakeTimer_           = 0.0f;
            shakingDigitIndex_    = -1;
            rankSEPlayed_         = false;
        }


        void ResultMenu::DebugSetNumber(int value)
        {
            UINumberSprite* digits[] = { levelDigit_, enemyDigit1_, enemyDigit2_, enemyDigit3_, scoreDigit_ };
            for (auto* d : digits) {
                if (d) {
                    d->SetNumber(value);
                    d->isDraw = true;
                    d->transform.localScale = Vector3::One;
                    d->transform.UpdateTransform();
                }
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
