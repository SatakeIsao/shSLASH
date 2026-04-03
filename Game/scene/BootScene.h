/**
 * BootScene.h
 * 起動画面のシーン
 * NOTE: デバッグ用でシーンの遷移を簡単にしている
 */
#pragma once
#include "IScene.h"

#if defined(APP_DEBUG)

enum class SceneKind : uint8_t
{
	Default,
	Debug = Default,
	Startup,
	GameClear,
	//GameOver,
	Max,
};


/** 起動シーン */
class BootScene : public IScene
{
	appScene(BootScene);


private:
	/** 遷移をリクエストする先のシーンID */
	uint32_t m_requestSceneId = INVALID_SCENE_ID;
	/** シーン選択表示用 */
	FontRender m_sceneText[static_cast<uint8_t>(SceneKind::Max)];
	/** どれを選択しているかわかるようにアイコン */
	SpriteRender m_selecterRender;
	int m_selectIndex = static_cast<uint8_t>(SceneKind::Default);


public:
	BootScene();
	virtual ~BootScene();

	virtual bool Start() override;
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;

	virtual bool RequestScene(uint32_t& id, float& waitTime)  override;
};

#endif