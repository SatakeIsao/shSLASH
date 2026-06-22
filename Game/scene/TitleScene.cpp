/**
 * TitleScene.cpp
 * タイトルシーン
 */
#include "stdafx.h"
#include "TitleScene.h"
#include "BattleScene.h"
#include "TutorialScene.h"
#include "title/TitleManager.h"
#include "sound/SoundManager.h"

#if defined(APP_DEBUG)
#include "DebugScene.h"
#endif // APP_DEBUG

TitleScene::TitleScene()
{}


TitleScene::~TitleScene()
{
	/** シーン終了時にマネージャーを終了 */
	app::title::TitleManager::Finalize();
}


bool TitleScene::Start()
{
	app::title::TitleManager::Initialize();
	app::title::TitleManager::Get().Start();
	app::SoundManager::Get().PlayBGM(static_cast<int>(app::SoundKind::Title));
	return true;
}


void TitleScene::Update()
{
	app::title::TitleManager::Get().Update();
}


void TitleScene::Render(RenderContext& rc)
{}


bool TitleScene::RequestScene(uint32_t& id, float& waitTime)
{
	if (app::title::TitleManager::Get().IsGameStartDecided())
	{
		id = BattleScene::ID();
		waitTime = 1.0f;
		return true;
	}

	if (app::title::TitleManager::Get().IsTutorialDecided())
	{
		id = TutorialScene::ID();
		waitTime = 1.0f;
		return true;
	}

	return false;
}