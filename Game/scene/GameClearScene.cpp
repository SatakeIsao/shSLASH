/**
 * GameClearScene.cpp
 * ゲームクリア画面を表示
 */

#include "stdafx.h"
#include "GameClearScene.h"
#include "TitleScene.h"


GameClearScene::GameClearScene()
{
}


GameClearScene:: ~GameClearScene()
{
}


bool GameClearScene::Start()
{
	m_spriteRender.Init("Assets/ui/gameclear/gameClear.DDS", MAX_SPRITE_WIDTH, MAX_SPRITE_HIGHT);
	return true;
}


void GameClearScene::Update()
{
	if (g_pad[0]->IsTrigger(enButtonDown))
	{
		m_requestSceneId = TitleScene::ID();
	}

	m_spriteRender.Update();
}


void GameClearScene::Render(RenderContext& rc)
{
	m_spriteRender.Draw(rc);
}

bool GameClearScene::RequestScene(uint32_t& id, float& waitTime)
{
	if (m_requestSceneId != INVALID_SCENE_ID)
	{
		id = m_requestSceneId;
		waitTime = 3.0f;
		return true;
	}
	return false;
}
