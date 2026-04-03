/**
 * DebugScene.h
 * デバッグ動作確認用シーン
 */
#pragma once
#include "IScene.h"

#if defined(APP_DEBUG)

class DebugScene : public IScene
{
	appScene(DebugScene);


private:



public:
	DebugScene();
	virtual ~DebugScene();

	virtual bool Start() override;
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;

	virtual bool RequestScene(uint32_t& id, float& waitTime)  override;

public:
	void Change();
	bool CanChange() const;

};

#endif // APP_DEBUG