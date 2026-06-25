/**
 * BattleScene.h
 * バトルシーン
 */
#pragma once
#include "IScene.h"
#include "ui/Layout.h"
#include "ui/GameOverSequence.h"

 /** バトルシーン */
class BattleScene : public IScene
{
	appScene(BattleScene);


private:
	uint32_t requestSceneId_ = INVALID_SCENE_ID;

	//
	// app::ui::Layout gameOverLayout_;
	app::ui::GameOverSequence* gameOverSequence_ = nullptr;
	bool isGameOver_ = false;

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

