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
            auto attachOvershoot = [&](UIDigit* digit) {
                if (!digit)return;
                app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(digit, Hash32("DigitOvershoot_Pop"));
                app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(digit, Hash32("DigitOvershoot_Settle"));
            };
            attachOvershoot(levelDigit_);
            attachOvershoot(enemyDigit1_);
            attachOvershoot(enemyDigit2_);
            attachOvershoot(enemyDigit3_);
            attachOvershoot(scoreDigit_);
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

            if (currentState_ >= SequenceState::Init && currentState_ <= SequenceState::ShowDigits) {
                if (g_pad[0]->IsTrigger(enButtonA)) {

                    if (scoreBoard_ && !scoreBoard_->isDraw) { scoreBoard_->isDraw = true; auto* anim = scoreBoard_->FindAnimation(Hash32("ScoreBoardSlideIn")); if (anim) anim->Play(); }
                    if (enemyIcon1_ && !enemyIcon1_->isDraw) { enemyIcon1_->isDraw = true; auto* anim = enemyIcon1_->FindAnimation(Hash32("EnemyIcon1Slide")); if (anim) anim->Play(); }
                    if (enemyIcon2_ && !enemyIcon2_->isDraw) { enemyIcon2_->isDraw = true; auto* anim = enemyIcon2_->FindAnimation(Hash32("EnemyIcon2Slide")); if (anim) anim->Play(); }
                    if (enemyIcon3_ && !enemyIcon3_->isDraw) { enemyIcon3_->isDraw = true; auto* anim = enemyIcon3_->FindAnimation(Hash32("EnemyIcon3Slide")); if (anim) anim->Play(); }

                    if (levelDigit_ && !levelDigit_->isDraw) { levelDigit_->isDraw = true; auto* anim = levelDigit_->FindAnimation(Hash32("DigitPop")); if (anim) anim->Play(); }
                    if (enemyDigit1_ && !enemyDigit1_->isDraw) { enemyDigit1_->isDraw = true; auto* anim = enemyDigit1_->FindAnimation(Hash32("DigitPop")); if (anim) anim->Play(); }
                    if (enemyDigit2_ && !enemyDigit2_->isDraw) { enemyDigit2_->isDraw = true; auto* anim = enemyDigit2_->FindAnimation(Hash32("DigitPop")); if (anim) anim->Play(); }
                    if (enemyDigit3_ && !enemyDigit3_->isDraw) { enemyDigit3_->isDraw = true; auto* anim = enemyDigit3_->FindAnimation(Hash32("DigitPop")); if (anim) anim->Play(); }
                    if (scoreDigit_ && !scoreDigit_->isDraw) { scoreDigit_->isDraw = true; auto* anim = scoreDigit_->FindAnimation(Hash32("DigitPop")); if (anim) anim->Play(); }

                    if (skipIcon_) skipIcon_->isDraw = false;

                    currentState_ = SequenceState::ShowRank;
                    stateTimer_ = 1.5f;
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
                        // 時間が来たら次の状態へ
                        currentState_ = SequenceState::SlideInScoreBoardAndResult;
                        
                        // 両方のアニメーションを同時に再生！
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

                //数字を順番に表示していく
                case SequenceState::ShowDigits:
                {
                    stateTimer_ -= g_gameTime->GetFrameDeltaTime();
                    
                    UIDigit* currentDigit = nullptr;
                    if (currentDigitStep_ == 0)currentDigit = levelDigit_;
                    else if (currentDigitStep_ == 1)currentDigit = enemyDigit1_;
                    else if (currentDigitStep_ == 2)currentDigit = enemyDigit2_;
                    else if (currentDigitStep_ == 3)currentDigit = enemyDigit3_;
					else if (currentDigitStep_ == 4)currentDigit = scoreDigit_;

                    if (currentDigit) {
                        if (!currentDigit->isDraw) {
                            currentDigit->isDraw = true;
                            auto* anim = currentDigit->FindAnimation(Hash32("DigitOvershoot_Pop"));
                            if (anim) anim->Play();
                        }

                        if (stateTimer_ <= 0.85f) {
                            auto* anim = currentDigit->FindAnimation(Hash32("DigitOvershoot_Settle"));
                            if (anim && !anim->IsPlay()) {
                                anim->Play();
                            }
                        }
                    }

                    if (stateTimer_ <= 0.0f) {
                        if (currentDigitStep_ == 4) {
                            if (skipIcon_)skipIcon_->isDraw = false;
							currentState_ = SequenceState::ShowRank;
							stateTimer_ = 1.5f;
                        }
                        else {
                            currentDigitStep_++;
                            stateTimer_ = 1.0f;
                        }
                    }
                    break;
                }

                case SequenceState::ShowRank:
                {
                    stateTimer_ -= g_gameTime->GetFrameDeltaTime();
                    if (stateTimer_ <= 0.0f) {
                        // TODO:ここにランクを表示する処理を書く
                        if (nextIcon_) {
                            nextIcon_->isDraw = true;
                            // もしNextにフェードインなどのアニメーションがあればここでPlay()
                        }

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
                        if (subMenu) subMenu->OnOpen();
                        return;
                    }
                    if (g_pad[0]->IsTrigger(enButtonA)) {
                        if (rankA_) rankA_->transform.localScale = Vector3::Zero;
                        if (rankB_) rankB_->transform.localScale = Vector3::Zero;
                        if (rankSFog_) rankSFog_->transform.localScale = Vector3::Zero;

                        if (rankS_) {
                            rankS_->transform.localScale = Vector3::One;
                            rankS_->transform.localRotation = Quaternion::Identity; // 角度をまっすぐに
                            auto* anim = rankS_->FindAnimation(Hash32("RankStamp"));
                            if (anim) anim->Play();
                        }
                        debugCurrentRank_ = 0;
                        sFogTimer_ = 0.0f; // Sランクのタイマーをリセット
                    }

                    // 【Bボタン】Aランクの再生
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

                    // 【Xボタン】Bランクの再生
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

                    // ==========================================
                    // Sランク演出
                    // ==========================================
                    if (debugCurrentRank_ == 0 && rankSFog_ && rankS_) {
                        sFogTimer_ += g_gameTime->GetFrameDeltaTime();
                        float fogScale = 1.0f + 0.1f * sinf(sFogTimer_ * 5.0f);
                        rankSFog_->transform.localScale = rankS_->transform.localScale * fogScale;

                        rankSFog_->color.w = 0.6f + 0.4f * sinf(sFogTimer_ * 5.0f);
                    }
                    // ==========================================
                    // Bランク演出
                    // ==========================================
                    if (debugCurrentRank_ == 2 && rankB_ && !isBRankTilted_) {
                        bRankTimer_ += g_gameTime->GetFrameDeltaTime();

                        if (bRankTimer_ >= 1.5f) {

                            // 0.5秒かけてゆっくり倒れる計算
                            float t = (bRankTimer_ - 1.5f) / 1.0f;

                            if (t < 1.0f) {
                                float currentAngle = -7.5f * (t * t);
                                rankB_->transform.localRotation.SetRotationDeg(Vector3(0.0f, 0.0f, 1.0f), currentAngle);
                            }
                            else {
                                rankB_->transform.localRotation.SetRotationDeg(Vector3(0.0f, 0.0f, 1.0f), -7.5f);
                                isBRankTilted_ = true; // ストッパー
                            }
                            rankB_->transform.UpdateTransform();
                        }
                    }
                    break;
                }
                case SequenceState::OpenMenu:
                {
                    // サブメニューの更新
                    if (subMenuLayout_) {
                        subMenuLayout_->Update();
                    }
                    break;
                }
            }
        }
        void ResultMenu::Render(RenderContext& rc) {
            // 元のリザルト画面を描画
            MenuBase::Render(rc);

            // サブメニュー展開中なら上に重ねて描画
            if (currentState_ == SequenceState::OpenMenu && subMenuLayout_) {
                subMenuLayout_->Render(rc);
            }
        }

        void ResultMenu::OnOpen() {
            // 開くときのアニメーション
        }

        void ResultMenu::OnClose() {
            // 閉じるときのアニメーション
        }
    }
}