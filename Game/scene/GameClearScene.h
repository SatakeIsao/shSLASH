/**
 * GameClearScene.h
 * ゲームクリアシーンクラス
 */
#pragma once
#include "IScene.h"


/** ゲームクリアシーン */
class GameClearScene : public IScene
{
	appScene(GameClearScene);


private:
	/** 遷移をリクエストする先のシーンID */
	uint32_t m_requestSceneId = INVALID_SCENE_ID;

	SpriteRender m_spriteRender;


public:
	GameClearScene();
	virtual ~GameClearScene();
	
	virtual bool Start() override;
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;

	virtual bool RequestScene(uint32_t& id, float& waitTime) override;
};

