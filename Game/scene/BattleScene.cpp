#include "stdafx.h"
#include "BattleScene.h"
#include "GameOverScene.h"
#include "GameClearScene.h"
#include "TitleScene.h"
#include "battle/BattleManager.h"
#include "ui/SoundOptionMenu.h"


namespace
{
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

	return true;
}


void BattleScene::Update()
{
	app::battle::BattleManager::Get().Update();
}


void BattleScene::Render(RenderContext& rc)
{
}


bool BattleScene::RequestScene(uint32_t& id, float& waitTime)
{
	//app::ui::SoundOptionMenu* sound;
	//sound = new app::ui::SoundOptionMenu;

	if (app::battle::BattleManager::Get().GetDeadTest())
	{
		id = GameOverScene::ID();
		waitTime = 3.0f;
		return true;
	}
	if (g_pad[0]->IsTrigger(enButtonRight))
	{
		id = GameClearScene::ID();
		waitTime = 3.0f;
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
