/**
 * BootScene.cpp
 * 起動画面のシーン
 */
#include "stdafx.h"
#include "TitleScene.h"
#include "BattleScene.h"
#include "title/TitleManager.h"

#if defined(APP_DEBUG)
#include "DebugScene.h"
#endif // APP_DEBUG

TitleScene::TitleScene()
{
}


TitleScene::~TitleScene()
{
	/**シーン終了時にマネージャーを破棄する*/
	app::title::TitleManager::Finalize();
}


bool TitleScene::Start()
{
	app::title::TitleManager::Initialize();
	app::title::TitleManager::Get().Start();
	return true;
}


void TitleScene::Update()
{
	app::title::TitleManager::Get().Update();
}


void TitleScene::Render(RenderContext& rc)
{
}


bool TitleScene::RequestScene(uint32_t& id, float& waitTime)
{
	if (app::title::TitleManager::Get().IsGameStartDecided())
	{
		id = BattleScene::ID();
		waitTime = 1.0f;
		return true;
	}


	return false;
}