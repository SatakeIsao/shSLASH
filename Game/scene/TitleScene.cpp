/**
 * BootScene.cpp
 * ‹N“®‰æ–Ê‚ÌƒV[ƒ“
 */
#include "stdafx.h"
#include "TitleScene.h"
#include "BattleScene.h"

#if defined(APP_DEBUG)
#include "DebugScene.h"
#endif // APP_DEBUG

TitleScene::TitleScene()
{
}


TitleScene::~TitleScene()
{
}


bool TitleScene::Start()
{
	backGroundRender_.Init("Assets/ui/title/background.dds", MAX_SPRITE_WIDTH, MAX_SPRITE_HIGHT);
	return true;
}


void TitleScene::Update()
{
	if (g_pad[0]->IsTrigger(enButtonA)) {
#if defined(APP_DEBUG)
		m_requestSceneId = BattleScene::ID();
#endif
	}

	backGroundRender_.Update();
}


void TitleScene::Render(RenderContext& rc)
{
	backGroundRender_.Draw(rc);
}


bool TitleScene::RequestScene(uint32_t& id, float& waitTime)
{
	if (m_requestSceneId != INVALID_SCENE_ID) {
		id = m_requestSceneId;
		waitTime = 3.0f;
		return true;
	}
	return false;
}