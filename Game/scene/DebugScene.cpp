/**
 * DebugScene.cpp
 * デバッグ動作確認用シーン
 */

#include "stdafx.h"
#if defined(APP_DEBUG)

#include "DebugScene.h"
#include "GameOverScene.h"
#include "GameClearScene.h"
#include "battle/BattleManager.h"
#include "ui/Layout.h"


namespace
{
	// @todo for test
	//static app::ui::Layout* testLayout;
}


DebugScene::DebugScene()
{
}


DebugScene::~DebugScene()
{
	app::battle::BattleManager::Finalize();
}


bool DebugScene::Start()
{
	app::battle::BattleManager::Initialize();

	app::battle::BattleManager::Get().Start();

	//testLayout = new app::ui::Layout();
	//testLayout->Initialize<app::ui::MenuBase>("Assets/ui/layout/testLayout.json");

	return true;
}


void DebugScene::Update()
{
	//testLayout->Update();
}


void DebugScene::Render(RenderContext& rc)
{
	/** DEBUG: テキストが表示されるか */
	//testLayout->Render(rc);
}


bool DebugScene::RequestScene(uint32_t& id, float& waitTime)
{
	if (g_pad[0]->IsTrigger(enButtonRight)) {
		id = GameClearScene::ID();
		waitTime = 3.0f;
		return true;
	}
	if (g_pad[0]->IsTrigger(enButtonLeft)) {
		id = GameOverScene::ID();
		waitTime = 3.0f;
		return true;
	}
	return false;
}


void DebugScene::Change()
{
}


bool DebugScene::CanChange() const
{
	if (g_pad[0]->IsTrigger(enButtonRight))
	{
		return true;
	}

	return false;
}

#endif // APP_DEBUG