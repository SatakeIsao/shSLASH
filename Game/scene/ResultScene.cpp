/**
 * GameClearScene.cpp
 * ゲームクリア画面を表示
 */

#include "stdafx.h"
#include "ResultScene.h"
#include "TitleScene.h"


ResultScene::ResultScene()
{
}


ResultScene:: ~ResultScene()
{
}


bool ResultScene::Start()
{
	m_spriteRender.Init("Assets/ui/result/test_gameResult.DDS", MAX_SPRITE_WIDTH, MAX_SPRITE_HIGHT);
	return true;
}


void ResultScene::Update()
{
	if (g_pad[0]->IsTrigger(enButtonDown))
	{
		m_requestSceneId = TitleScene::ID();
	}

	m_spriteRender.Update();
}


void ResultScene::Render(RenderContext& rc)
{
	m_spriteRender.Draw(rc);
}


bool ResultScene::RequestScene(uint32_t& id, float& waitTime)
{
	if (m_requestSceneId != INVALID_SCENE_ID)
	{
		id = m_requestSceneId;
		waitTime = 3.0f;
		return true;
	}
	return false;
}
