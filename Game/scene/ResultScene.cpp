/**
 * GameClearScene.cpp
 * ゲームクリア画面を表示
 */

#include "stdafx.h"
#include "ResultScene.h"
#include "TitleScene.h"
#include "ui/ResultMenu.h"

ResultScene::ResultScene()
{
}


ResultScene:: ~ResultScene()
{
	app::camera::CameraManager::Get().Unregister(Hash32("ResultCamera"));
}


bool ResultScene::Start()
{
	layout_.Initialize<app::ui::ResultMenu>("Assets/ui/layout/resultLayout.json");


	// ==========================================
	// カメラ
	// ==========================================

	app::camera::CameraData camData;
	camData.position = Vector3(200.0f, 25.0f, -50.0f); // カメラを置く位置
	camData.target = Vector3(80.0f, 40.0f, -50.0f);      // カメラが見つめる中心点

	resultCamera_ = std::make_shared<app::camera::GameCamera>();
	resultCamera_->SetState(camData);

	app::camera::CameraManager::Get().Register(Hash32("ResultCamera"), resultCamera_);
	app::camera::CameraManager::Get().SwitchCamera(Hash32("ResultCamera"), 0.0f);


	// ==========================================
	// モデルとアニメーションのロード
	// ==========================================

	animationClips_.Create(1);
	animationClips_[0].Load("Assets/animData/player/playerIdle.tka");
	animationClips_[0].SetLoopFlag(true);

	playerModel_ = std::make_unique<ModelRender>();
	playerModel_->Init("Assets/ModelData/player/player.tkm", animationClips_.data(), animationClips_.size());

	modelTransform_.localPosition = Vector3(80.0f, 0.0f, 0.0f);
	modelTransform_.localScale = Vector3(1.0f, 1.0f, 1.0f);
	modelTransform_.localRotation.SetRotationDeg(Vector3::Up, 90.0f);

	modelTransform_.UpdateTransform();
	
	return true;
}


void ResultScene::Update()
{
	if (g_pad[0]->IsTrigger(enButtonDown))
	{
		m_requestSceneId = TitleScene::ID();
	}
	app::camera::CameraManager::Get().Update(g_gameTime->GetFrameDeltaTime());

	if (playerModel_) {
		playerModel_->SetTRS(modelTransform_.position, modelTransform_.rotation, modelTransform_.scale);
		playerModel_->Update();
	}

	layout_.Update();
}


void ResultScene::Render(RenderContext& rc)
{
	if (playerModel_) {
		playerModel_->Draw(rc); 
	}
	layout_.Render(rc);
}


bool ResultScene::RequestScene(uint32_t& id, float& waitTime)
{
	if (m_requestSceneId != INVALID_SCENE_ID)
	{
		id = m_requestSceneId;
		waitTime = 3.0f;
		return true;
	}
	return false;
}
