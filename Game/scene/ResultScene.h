/**
 * ResultScene.h
 * ゲームクリアシーンクラス
 */
#pragma once
#include "IScene.h"


/** ゲームクリアシーン */
class ResultScene : public IScene
{
	appScene(ResultScene);


private:
	/** 遷移をリクエストする先のシーンID */
	uint32_t m_requestSceneId = INVALID_SCENE_ID;

	SpriteRender m_spriteRender;


public:
	ResultScene();
	virtual ~ResultScene();
	
	virtual bool Start() override;
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;

	virtual bool RequestScene(uint32_t& id, float& waitTime) override;
};

