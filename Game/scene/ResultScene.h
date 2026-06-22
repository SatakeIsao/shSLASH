/**
 * ResultScene.h
 * ゲームクリアシーンクラス
 */
#pragma once
#include "IScene.h"
#include "ui/Layout.h"
#include "camera/CameraManager.h"
#include "camera/CameraController.h"

namespace nsK2EngineLow { class SkyCube; }

 /** ゲームクリアシーン */
class ResultScene : public IScene
{
	appScene(ResultScene);


private:
	/** 遷移をリクエストする先のシーンID */
	uint32_t m_requestSceneId = INVALID_SCENE_ID;

	//SpriteRender m_spriteRender;
	app::ui::Layout layout_;

	nsK2EngineLow::SkyCube* skyCube_ = nullptr;
	std::unique_ptr<ModelRender> stageModel_;
	std::unique_ptr<ModelRender> playerModel_;
	app::memory::Array<AnimationClip> animationClips_;
	app::math::Transform modelTransform_;

	std::shared_ptr<app::camera::GameCamera> resultCamera_;

	float cameraTime_             = 0.0f;
	bool  beginnerShakeTriggered_ = false;
	bool  masterShakeTriggered_   = false;
	bool  beginnerIdlePlaying_        = false;
	bool  beginnerIdleSlowMode_       = false;
	float beginnerIdlePhase_          = 0.0f;
	float beginnerIdleTime_           = 0.0f;
	float beginnerIdleTotalDuration_  = 0.0f;

	bool  eliteIdlePlaying_           = false;

	bool  masterIdlePlaying_          = false;
	bool  masterIdleSlowMode_         = false;
	float masterIdlePhase_            = 0.0f;
	float masterIdleTime_             = 0.0f;
	float masterIdleTotalDuration_    = 0.0f;

	int debugRankIndex_  = 0;
	int debugTestNumber_ = 0;

	void UpdateCameraWork(float deltaTime);

public:
	ResultScene();
	virtual ~ResultScene();

	virtual bool Start() override;
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;

	virtual bool RequestScene(uint32_t& id, float& waitTime) override;
};

