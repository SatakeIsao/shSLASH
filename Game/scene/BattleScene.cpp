#include "stdafx.h"
#include "BattleScene.h"
#include "GameOverScene.h"
#include "ResultScene.h"
#include "TitleScene.h"
#include "battle/BattleManager.h"
#include "core/PauseManager.h"
#include "ui/SoundOptionMenu.h"
#include "ui/GameOverSequence.h"
#include "sound/SoundManager.h"

namespace app
{
	class BattleGameOverMenu : public ui::MenuBase {
	public:
		void InitializeLogic() override {}
		void Update() override { MenuBase::Update(); }
	};
}



BattleScene::BattleScene()
{
}


BattleScene::~BattleScene()
{
	app::SoundManager::Get().StopAllSE();
	app::battle::BattleManager::Finalize();
	DeleteGO(gameOverSequence_);
}


bool BattleScene::Start()
{
	g_sceneLight->SetBattleLighting();
	if (!app::battle::BattleManager::IsAvailable()) {
		app::battle::BattleManager::Initialize();
	}
	app::battle::BattleManager::Get().Start();
	gameOverSequence_ = NewGO<app::ui::GameOverSequence>(static_cast<uint8_t>(ObjectPriority::GameOverUI));
	return true;
}


void BattleScene::Update()
{
	app::battle::BattleManager::Get().Update();

	if (!isGameOver_ && app::battle::BattleManager::Get().IsPlayerDead())
	{
		isGameOver_ = true;
		app::battle::BattleManager::Get().SetGameOverFreeze(true);
		if (gameOverSequence_) gameOverSequence_->StartSequence();
	}
}


void BattleScene::Render(RenderContext& rc)
{
	if (app::battle::BattleManager::IsAvailable())
	{
		app::battle::BattleManager::Get().Render(rc);
	}
}


bool BattleScene::RequestScene(uint32_t& id, float& waitTime)
{
	//app::ui::SoundOptionMenu* sound;
	//sound = new app::ui::SoundOptionMenu;

	if (isGameOver_)
	{
		if (gameOverSequence_)
		{
			if (gameOverSequence_->IsRetryDecided())
			{
				id = BattleScene::ID();
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
			id = BattleScene::ID();
			waitTime = 1.0f;
			return true;
		}
	}

#if defined(APP_DEBUG)
	/** デバッグ: LB2+Downでゲームオーバーシーケンスを強制再生 */
	if (g_pad[0]->IsPress(enButtonLB2)
		&& g_pad[0]->IsTrigger(enButtonDown))
	{
		isGameOver_ = true;
		app::battle::BattleManager::Get().SetGameOverFreeze(true);
		if (gameOverSequence_) {
			gameOverSequence_->StartSequence();
		}
		return false;
	}
#endif // APP_DEBUG

	/** タイマーが0になったら */
	if (app::battle::BattleManager::Get().IsTimeUpFinished())
	{
		id = ResultScene::ID();
		waitTime = 0.5f;
		return true;
	}


	/*if (sound->IsTitle())
	{
		id = TitleScene::ID();
		waitTime = 3.0f;
		return true;
	}*/


	if (requestSceneId_ != INVALID_SCENE_ID)
	{
		id = requestSceneId_;
		waitTime = 1.0f;
		return true;
	}

	return false;
}


void BattleScene::Change()
{
}


bool BattleScene::CanChange() const
{
	return true;
}
