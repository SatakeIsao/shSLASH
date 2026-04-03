/**
 * StartupScene.h
 * スタートアップシーン
 */
#pragma once
#include "IScene.h"


 /** スタートアップシーン */
class StartupScene : public IScene
{
	appScene(StartupScene);


private:
	SpriteRender* m_spriteRender;
	float m_elapsedTime = 0.0f;
	float m_changeTime = 0.0f;
	float m_shortChangeTime = 0.0f;
	int m_currentIndex = 0;


public:
	StartupScene();
	virtual ~StartupScene();

	virtual bool Start() override;
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;

	virtual bool RequestScene(uint32_t& id, float& waitTime)  override;


private:
	void Change();

	bool CanChange() const;
};

