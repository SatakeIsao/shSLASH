/**
 * TitleScene.h
 * タイトルシーン
 */
#pragma once
#include "IScene.h"


/** タイトルシーン */
class TitleScene : public IScene
{
	appScene(TitleScene);


private:
	/** 遷移をリクエストする先のシーンID */
	uint32_t m_requestSceneId = INVALID_SCENE_ID;

	SpriteRender backGroundRender_;


public:
	TitleScene();
	virtual ~TitleScene();

	virtual bool Start() override;
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;

	virtual bool RequestScene(uint32_t& id, float& waitTime)  override;
};