/** 
 * GameOverScene.h
 * ゲームオーバーシーンクラス
 */
#pragma once
#include "IScene.h"


/** ゲームオーバーシーン */
class GameOverScene : public IScene
{
	appScene(GameOverScene);


private:
	/** 遷移をリクエストする先のシーンID */
	uint32_t m_requestSceneId = INVALID_SCENE_ID;

	SpriteRender backGroundRender_;


public:
	GameOverScene();
	virtual ~GameOverScene();

	virtual bool Start() override;
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;

	virtual bool RequestScene(uint32_t& id, float& waitTime)  override;

	void Change();
	bool CanChange() const;
};

