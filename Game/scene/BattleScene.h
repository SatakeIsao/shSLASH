/**
 * BattleScene.h
 * バトルシーン
 */
#pragma once
#include "IScene.h"


/** バトルシーン */
class BattleScene : public IScene
{
	appScene(BattleScene);


private:
	uint32_t requestSceneId_ = INVALID_SCENE_ID;

public:
	BattleScene();
	virtual ~BattleScene();

	virtual bool Start() override;
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;

	virtual bool RequestScene(uint32_t& id, float& waitTime);


public:
	void Change();
	bool CanChange() const;
};

