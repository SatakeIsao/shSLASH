#include "stdafx.h"
#include "BattleScene.h"
#include "GameOverScene.h"
#include "ResultScene.h"
#include "TitleScene.h"
#include "battle/BattleManager.h"
#include "core/PauseManager.h"
#include "ui/SoundOptionMenu.h"
#include "ui/GameOverSequence.h"

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
	app::battle::BattleManager::Finalize();
}


bool BattleScene::Start()
{
	app::battle::BattleManager::Initialize();
	app::battle::BattleManager::Get().Start();

	gameOverSequence_ = std::make_unique<app::ui::GameOverSequence>();

	return true;
}


void BattleScene::Update()
{
	app::battle::BattleManager::Get().Update();

	if (isGameOver_) {
		gameOverSequence_->Update();
	}
}


void BattleScene::Render(RenderContext& rc)
{
	if (isGameOver_) {
		gameOverSequence_->Render(rc);
	}
}


bool BattleScene::RequestScene(uint32_t& id, float& waitTime)
{
	//app::ui::SoundOptionMenu* sound;
	//sound = new app::ui::SoundOptionMenu;

	if (isGameOver_)
	{
		if (gameOverSequence_ && gameOverSequence_->IsReturnTitleDecided())
		{
			id = TitleScene::ID();
			waitTime = 1.0f;
			return true;
		}
		return false;
	}

	/** Playerのダウンモーションが再生終了したら */
	if (g_pad[0]->IsPress(enButtonLB2)
		&& g_pad[0]->IsTrigger(enButtonDown))
		//if (app::battle::BattleManager::Get().GetDeadTest())
	{
		isGameOver_ = true;
		if (gameOverSequence_) {
			gameOverSequence_->StartSequence();
		}
		/*id = GameOverScene::ID();
		waitTime = 3.0f;*/
		return false;
	}

	/** タイマーが0になったら */
	if (g_pad[0]->IsPress(enButtonLB2)
		&& g_pad[0]->IsTrigger(enButtonRight))
	{
		id = ResultScene::ID();
		waitTime = 3.0f;
		return true;
	}


	/*if (sound->IsTitle())
	{
		id = TitleScene::ID();
		waitTime = 3.0f;
		return true;
	}*/

	if (app::core::PauseManager::Get().IsReturnToTitleRequested())
	{
		id = TitleScene::ID();
		waitTime = 3.0f;
		return true;
	}

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
