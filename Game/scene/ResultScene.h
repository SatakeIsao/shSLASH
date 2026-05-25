/**
 * ResultScene.h
 * ゲームクリアシーンクラス
 */
#pragma once
#include "IScene.h"
#include "ui/Layout.h"
#include "camera/CameraManager.h"
#include "camera/CameraController.h"


 /** ゲームクリアシーン */
class ResultScene : public IScene
{
	appScene(ResultScene);


private:
	/** 遷移をリクエストする先のシーンID */
	uint32_t m_requestSceneId = INVALID_SCENE_ID;

	//SpriteRender m_spriteRender;
	app::ui::Layout layout_;

	std::unique_ptr<ModelRender> playerModel_;
	app::memory::Array<AnimationClip> animationClips_;
	app::math::Transform modelTransform_;

	std::shared_ptr<app::camera::GameCamera> resultCamera_;

public:
	ResultScene();
	virtual ~ResultScene();

	virtual bool Start() override;
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;

	virtual bool RequestScene(uint32_t& id, float& waitTime) override;
};

