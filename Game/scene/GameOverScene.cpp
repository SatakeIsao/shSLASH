/**
 * GameOverScene.cpp
 * ゲームオーバー画面を起動
 */

#include "stdafx.h"
#include "GameOverScene.h"
#include "TitleScene.h"

namespace app {
	class GameOverMenu : public ui::MenuBase {
	public:
		void InitializeLogic() override {}

		void Update() override {
			MenuBase::Update();
		}
	};
}

GameOverScene::GameOverScene()
{
}


GameOverScene:: ~GameOverScene()
{
}


bool GameOverScene::Start()
{
	//backGroundRender_.Init("Assets/ui/gameover/gameOver.DDS", MAX_SPRITE_WIDTH, MAX_SPRITE_HIGHT);
	//layout_.Initialize<app::GameOverMenu>("Assets/ui/layout/gameOverLayout.json");
	return true;
}


void GameOverScene::Update()
{
	if (g_pad[0]->IsTrigger(enButtonDown))
	{
		m_requestSceneId = TitleScene::ID();
	}
	//backGroundRender_.Update();
	layout_.Update();
}


void GameOverScene::Render(RenderContext& rc)
{
	//backGroundRender_.Draw(rc);
	layout_.Render(rc);
}


bool GameOverScene::RequestScene(uint32_t& id, float& waitTime)
{
	if (m_requestSceneId != INVALID_SCENE_ID)
	{
		id = m_requestSceneId;
		waitTime = 3.0f;
		return true;
	}
	return false;
}


void GameOverScene::Change()
{
}


bool GameOverScene::CanChange() const
{
	if (g_pad[0]->IsTrigger(enButtonRight))
	{
		return true;
	}
	return false;
}