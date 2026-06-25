#include "stdafx.h"
#include "TutorialScene.h"
#include "TitleScene.h"
#include "TutorialConfirmObject.h"
#include "battle/BattleManager.h"
#include "core/PauseManager.h"
#include "ui/GameOverSequence.h"
#include "ui/TutorialReturnConfirmMenu.h"
#include "sound/SoundManager.h"


TutorialScene::TutorialScene()
{
}


TutorialScene::~TutorialScene()
{
    app::SoundManager::Get().StopAllSE();
    DeleteGO(gameOverSequence_);
    if (confirmObject_)
    {
        DeleteGO(confirmObject_);
        confirmObject_ = nullptr;
    }
    if (tutorialUI_)
    {
        DeleteGO(tutorialUI_);
        tutorialUI_ = nullptr;
    }
    if (tutorialPauseHint_)
    {
        DeleteGO(tutorialPauseHint_);
        tutorialPauseHint_ = nullptr;
    }
    app::battle::BattleManager::Finalize();
}


bool TutorialScene::Start()
{
    g_sceneLight->SetBattleLighting();
    if (!app::battle::BattleManager::IsAvailable()) {
        app::battle::BattleManager::Initialize();
        app::battle::BattleManager::Get().SetTutorialMode(true);
        app::battle::BattleManager::Get().SetTutorialNoDamage(true);
    }
    app::battle::BattleManager::Get().Start();
    app::battle::BattleManager::Get().SetPlayerInputEnabled(false);

    gameOverSequence_ = NewGO<app::ui::GameOverSequence>(static_cast<uint8_t>(ObjectPriority::GameOverUI));

    tutorialUI_ = NewGO<app::ui::TutorialUIObject>(
        static_cast<uint8_t>(ObjectPriority::PlayerUI));

    tutorialPauseHint_ = NewGO<app::ui::TutorialPauseHintObject>(
        static_cast<uint8_t>(ObjectPriority::TutorialPauseHint));

    confirmLayout_ = std::make_unique<app::ui::Layout>();
    confirmLayout_->Initialize<app::ui::TutorialReturnConfirmMenu>(
        "Assets/ui/layout/tutorialReturnConfirmLayout.json");

    confirmObject_ = NewGO<TutorialConfirmObject>(
        static_cast<uint8_t>(ObjectPriority::ConfirmUI));

    return true;
}


void TutorialScene::Update()
{
    app::battle::BattleManager::Get().Update();

    const bool isPaused = app::core::PauseManager::IsAvailable()
                       && app::core::PauseManager::Get().IsPause();

    if (!isPaused && tutorialUI_)
    {
        if (!app::battle::BattleManager::Get().IsPlayerInputEnabled())
        {
            if (tutorialUI_->IsPlayerInputAllowed())
                app::battle::BattleManager::Get().SetPlayerInputEnabled(true);
        }

        // Aボタン進行メッセージ中だけ溜め攻撃を抑制する
        const bool suppressCharge = !tutorialUI_->IsPlayerInputAllowed();
        app::battle::BattleManager::Get().SetSuppressChargeAttackInput(suppressCharge);

        // 実践フェーズ前はリスポーン有効、フェーズ到達後は無効
        const bool practicePhase = tutorialUI_->IsPracticePhaseReached();
        app::battle::BattleManager::Get().SetTutorialRespawnEnabled(!practicePhase);

        // 実践フェーズ到達後はダメージを有効にする
        if (practicePhase)
            app::battle::BattleManager::Get().SetTutorialNoDamage(false);
    }

    // プレイヤーがDeadステートに遷移したらゲームオーバー開始
    if (!isGameOver_ && !isAllEnemiesDefeated_ && !isSkipRequested_)
    {
        if (app::battle::BattleManager::Get().IsPlayerDead())
        {
            isGameOver_ = true;
            app::battle::BattleManager::Get().SetGameOverFreeze(true);
            if (gameOverSequence_) gameOverSequence_->StartSequence();
        }
    }

    if (!isGameOver_ && !isAllEnemiesDefeated_ && !isSkipRequested_)
    {
        if (app::core::PauseManager::IsAvailable())
        {
            auto& pause = app::core::PauseManager::Get();
            if (pause.IsInPauseMenu())
            {
                if (!isConfirmOpen_ && g_pad[0]->IsTrigger(enButtonLeft))
                {
                    isConfirmOpen_ = true;
                    pause.SetCanOpenOption(false);
                    if (confirmObject_) confirmObject_->Activate(confirmLayout_.get());
                    auto* menu = dynamic_cast<app::ui::TutorialReturnConfirmMenu*>(
                        confirmLayout_->GetMenu());
                    if (menu) menu->ResetState();
                }
            }
        }

        if (isConfirmOpen_ && confirmLayout_)
        {
            bool isPauseActive = app::core::PauseManager::IsAvailable()
                && app::core::PauseManager::Get().IsPause();
            if (!isPauseActive)
            {
                isConfirmOpen_ = false;
                app::core::PauseManager::Get().SetCanOpenOption(true);
                if (confirmObject_) confirmObject_->Deactivate();
            }
            else
            {
                confirmLayout_->Update();
                auto* menu = dynamic_cast<app::ui::TutorialReturnConfirmMenu*>(
                    confirmLayout_->GetMenu());
                if (menu)
                {
                    if (menu->IsYesDecided())
                    {
                        isSkipRequested_ = true;
                    }
                    else if (menu->IsNoDecided())
                    {
                        isConfirmOpen_ = false;
                        if (app::core::PauseManager::IsAvailable())
                            app::core::PauseManager::Get().SetCanOpenOption(true);
                        if (confirmObject_) confirmObject_->Deactivate();
                    }
                }
            }
        }

        // 実践フェーズに入った後だけ全滅判定を行う
        const bool practicePhase = tutorialUI_ && tutorialUI_->IsPracticePhaseReached();
        if (practicePhase && app::battle::BattleManager::Get().IsTutorialAllEnemiesDefeated())
        {
            // CLEARアニメーションを起動し、完了後にシーン遷移
            if (tutorialUI_) tutorialUI_->TriggerFinalClear();
        }
        if (tutorialUI_ && tutorialUI_->IsFinalClearDone())
        {
            isAllEnemiesDefeated_ = true;
        }
    }

}


void TutorialScene::Render(RenderContext& rc)
{
    if (app::battle::BattleManager::IsAvailable())
    {
        app::battle::BattleManager::Get().Render(rc);
    }
}


bool TutorialScene::RequestScene(uint32_t& id, float& waitTime)
{
    if (isSkipRequested_ || isAllEnemiesDefeated_)
    {
        id = TitleScene::ID();
        waitTime = 1.0f;
        return true;
    }

    if (isGameOver_)
    {
        if (gameOverSequence_)
        {
            if (gameOverSequence_->IsRetryDecided())
            {
                id = TutorialScene::ID();
                waitTime = 1.0f;
                return true;
            }
            if (gameOverSequence_->IsReturnTitleDecided())
            {
                id = TitleScene::ID();
                waitTime = 1.0f;
                return true;
            }
            if (gameOverSequence_->IsExitDecided())
            {
                PostQuitMessage(0);
                return false;
            }
        }
        return false;
    }

    if (app::core::PauseManager::IsAvailable())
    {
        auto& pause = app::core::PauseManager::Get();
        if (pause.IsReturnToTitleRequested())
        {
            id = TitleScene::ID();
            waitTime = 1.0f;
            return true;
        }
        if (pause.IsRetryRequested())
        {
            id = TutorialScene::ID();
            waitTime = 1.0f;
            return true;
        }
    }

    return false;
}
