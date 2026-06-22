/**
 * GameClearScene.cpp
 * ゲームクリア画面を表示
 */

#include "stdafx.h"
#include "ResultScene.h"
#include "TitleScene.h"
#include "BattleScene.h"
#include "ui/ResultMenu.h"
#include "GameResultData.h"

namespace
{
	constexpr float kIntroDuration    = 1.5f;    // イントロ秒数
	constexpr float kMidRadius        = 125.0f;  // イントロ終端のカメラ距離
	constexpr float kMidHeight        = 28.0f;   // イントロ終端のカメラ高さ
	constexpr float kCamAngle         = -0.395f; // カメラのXZ角度（プレイヤー正面やや右）
	constexpr float kLookTargetY      = 45.0f;   // 通常注視点の高さ（胸元あたり）

	// BEGINNER 顔クローズアップ
	constexpr float kFaceHeight        = 50.0f;  // 顔の高さ（要調整）
	constexpr float kFaceCamRadius     = 45.0f;  // 顔アップ時のカメラ距離（要調整）
	constexpr float kFaceZoomDelay     = 0.5f;   // ズームイン開始までの待機時間
	constexpr float kFaceZoomDuration  = 3.0f;   // 顔へのズームイン所要時間


	// ELITE カメラ
	constexpr float kEliteCamYOffset    = 10.0f;    // 顔アップ時のY上げ量
	constexpr float kEliteCamRadius     = 35.0f;    // 顔アップ時のカメラ距離
	constexpr float kEliteHoldDuration  = 0.5f;     // 旋回前のミディアムショット静止時間
	constexpr float kEliteSwingAngle    = 3.14159f; // 旋回量（後ろから180°）
	constexpr float kEliteSwingTargetY  = 70.0f;    // 旋回開始時の注視点Y（足元を映さない）
	constexpr float kEliteSwingDuration = 4.0f;     // 旋回＋ズーム時間

	// MASTER 旋回・クローズアップ
	constexpr float kMasterMidHeight     = 33.0f;   // MASTER１段階目のカメラ高さ（要調整）
	constexpr float kMasterMidTargetY    = 50.0f;   // MASTER１段階目の注視点Y（要調整）
	constexpr float kMasterSwingDelta    = 0.608f;  // 旋回量（約40度）
	constexpr float kMasterSwingDuration = 1.5f;    // 旋回時間
	constexpr float kMasterFaceCamRadius = 80.0f;   // 顔アップ時のカメラ距離（要調整）

	// フレーミング寄せ
	constexpr float kFramingMaxOffset       =  10.0f; // 左寄せの最大量（BEGINNER・負=左）
	constexpr float kMasterFramingMaxOffset =  40.0f; // 右寄せの最大量（MASTER）
	constexpr float kEliteFramingOffset     = 10.0f; // 左寄せの最大量（ELITE・負=左）
	constexpr float kFramingDuration        =   4.0f; // 寄せ完了までの時間

	// cos 波で位相を駆動する ping-pong: time = (1 - cos(phase)) / 2 * totalDuration
	// 方向転換点(phase=0,π,2π)で速度が自然にゼロになるためクールタイムなし
	constexpr float kPingPongPI              = 3.14159265f; // 半周期（順再生1回分）
	constexpr float kPingPongTwoPI           = 6.28318530f; // 全周期
	constexpr float kMasterIdlePhaseRate     = 1.0f;        // rad/s：1段階目の位相速度
	constexpr float kMasterIdleSlowPhaseRate = 1.0f;        // rad/s：2段階目以降

	float EaseOutCubic(float t)
	{
		float s = 1.0f - t;
		return 1.0f - s * s * s;
	}

	float EaseInOutCubic(float t)
	{
		return t < 0.5f
			? 4.0f * t * t * t
			: 1.0f - powf(-2.0f * t + 2.0f, 3.0f) * 0.5f;
	}
}

ResultScene::ResultScene()
{
}


ResultScene:: ~ResultScene()
{
	app::camera::CameraManager::Get().Unregister(Hash32("ResultCamera"));
	app::camera::CameraManager::Get().SetScreenEffectActive(false);
	if (skyCube_) {
		DeleteGO(skyCube_);
	}
}


bool ResultScene::Start()
{
	g_renderingEngine->SetMotionBlurEnabled(false);
	app::camera::CameraManager::Get().SetMotionBlurEnabled(false);
	app::camera::CameraManager::Get().SetDepthOfFieldEnabled(true);
	app::camera::CameraManager::Get().SetScreenEffectActive(true);
	app::camera::CameraManager::Get().SetScreenEffectFocusWorldPos(Vector3(80.0f, 0.0f, 0.0f));

	g_sceneLight->SetResultLighting();

	layout_.Initialize<app::ui::ResultMenu>("Assets/ui/layout/resultLayout.json");


	// ==========================================
	// カメラ
	// ==========================================

	app::camera::CameraData camData;
	camData.position = Vector3(350.0f, 130.0f, 30.0f);   // イントロ開始位置（遠景・高台）
	camData.target   = Vector3(80.0f, kLookTargetY, 0.0f);

	resultCamera_ = std::make_shared<app::camera::GameCamera>();
	resultCamera_->SetState(camData);

	app::camera::CameraManager::Get().Register(Hash32("ResultCamera"), resultCamera_);
	app::camera::CameraManager::Get().SwitchCamera(Hash32("ResultCamera"), 0.0f);


	// ==========================================
	// モデルとアニメーションのロード
	// ==========================================

	// クリップインデックスはRankのenum値に対応 (Master=0, Elite=1, Beginner=2)
	animationClips_.Create(6);
	animationClips_[0].Load("Assets/animData/player/playerMaster.tka");
	animationClips_[0].SetLoopFlag(false);
	animationClips_[1].Load("Assets/animData/player/playerElite.tka");
	animationClips_[1].SetLoopFlag(false);
	animationClips_[2].Load("Assets/animData/player/playerBeginner_short.tka");
	animationClips_[2].SetLoopFlag(false);
	animationClips_[3].Load("Assets/animData/player/result/resultBegginer_Idle.tka");
	animationClips_[3].SetLoopFlag(true);
	animationClips_[4].Load("Assets/animData/player/result/resultElite_Idle.tka");
	animationClips_[4].SetLoopFlag(true);
	animationClips_[5].Load("Assets/animData/player/result/resultMaster_Idle.tka");
	animationClips_[5].SetLoopFlag(true);

	// スカイキューブ
	skyCube_ = NewGO<nsK2EngineLow::SkyCube>(0, "skycube");
	skyCube_->SetLuminance(1.0f);
	skyCube_->SetScale(300.0f);
	skyCube_->SetPosition({ 1000.0f, 0.0f, 1000.0f });
	skyCube_->SetType((nsK2EngineLow::EnSkyCubeType)enSkyCubeType_NightToon_2);

	// ステージモデル
	stageModel_ = std::make_unique<ModelRender>();
	stageModel_->Init("Assets/ModelData/stage/stage.tkm", nullptr, 0, enModelUpAxisY, false, true,
	                  "Assets/Shader/modelWall.fx", "Assets/Shader/model_gbuffer_wall.fx");
	stageModel_->SetTRS(Vector3(0.0f, 135.50f, 0.0f), Quaternion::Identity, Vector3(5.0f, 5.0f, 5.0f));
	stageModel_->Update();

	playerModel_ = std::make_unique<ModelRender>();
	playerModel_->Init("Assets/ModelData/player/player.tkm", animationClips_.data(), animationClips_.size());

	debugRankIndex_ = static_cast<int>(app::GameResultData::Get().CalcRank());
	playerModel_->PlayAnimation(debugRankIndex_, 0.0f);

	modelTransform_.localPosition = Vector3(80.0f, 0.0f, 0.0f);
	modelTransform_.localScale = Vector3(1.0f, 1.0f, 1.0f);
	modelTransform_.localRotation.SetRotationDeg(Vector3::Up, 90.0f);

	modelTransform_.UpdateTransform();
	
	return true;
}


void ResultScene::UpdateCameraWork(float deltaTime)
{
	cameraTime_ += deltaTime;

	const Vector3& playerPos = modelTransform_.localPosition;

	// イントロ終端のミディアムショット位置・注視点
	const Vector3 midPos = playerPos + Vector3(
		cosf(kCamAngle) * kMidRadius,
		kMidHeight,
		sinf(kCamAngle) * kMidRadius
	);
	const Vector3 midTarget = playerPos + Vector3(0.0f, kLookTargetY, 0.0f);

	// MASTERはイントロを1秒短縮して旋回を早める
	const float introDuration = (debugRankIndex_ == 0) ? (kIntroDuration - 1.0f) : kIntroDuration;

	app::camera::CameraData camData;

	if (cameraTime_ < introDuration && debugRankIndex_ != 1)
	{
		// フェーズ1: 遠景 → ミディアムショット（ELITE以外）
		const float t = EaseOutCubic(cameraTime_ / introDuration);

		const bool   isMaster   = (debugRankIndex_ == 0);
		const float  endHeight  = isMaster ? kMasterMidHeight  : kMidHeight;
		const float  endTargetY = isMaster ? kMasterMidTargetY : kLookTargetY;
		const Vector3 introEnd    = playerPos + Vector3(cosf(kCamAngle) * kMidRadius, endHeight,  sinf(kCamAngle) * kMidRadius);
		const Vector3 introTarget = playerPos + Vector3(0.0f, endTargetY, 0.0f);

		const Vector3 introStart = Vector3(350.0f, 130.0f, 30.0f);
		camData.position.Lerp(t, introStart, introEnd);
		camData.target = introTarget;
	}
	else if (debugRankIndex_ == 2) // BEGINNER: 顔へのズームイン
	{
		const float zoomStart = kIntroDuration + kFaceZoomDelay;

		if (cameraTime_ < zoomStart)
		{
			// 待機: ミディアムショットで静止（ズーム0.2秒前にシェイク）
			if (!beginnerShakeTriggered_ && cameraTime_ >= zoomStart - 0.2f)
			{
				resultCamera_->StartShake(app::camera::ShakeSize::Small);
				beginnerShakeTriggered_ = true;
			}
			camData.position = midPos;
			camData.target   = midTarget;
		}
		else
		{

			const float t      = min((cameraTime_ - zoomStart) / kFaceZoomDuration, 1.0f);
			const float easedT = EaseOutCubic(t);

			const Vector3 faceTarget = playerPos + Vector3(0.0f, kFaceHeight, 0.0f);
			const Vector3 faceCamPos = playerPos + Vector3(
				cosf(kCamAngle) * kFaceCamRadius,
				kFaceHeight,
				sinf(kCamAngle) * kFaceCamRadius
			);

			camData.position.Lerp(easedT, midPos, faceCamPos);
			camData.target.Lerp(easedT, midTarget, faceTarget);
		}
	}
	else if (debugRankIndex_ == 0) // MASTER: 旋回 → 顔クローズアップ
	{
		const float swingElapsed = cameraTime_ - introDuration;
		const float swingT       = min(swingElapsed / kMasterSwingDuration, 1.0f);
		const float swingAngle   = kCamAngle + kMasterSwingDelta * EaseOutCubic(swingT);

		// 旋回終端の角度・位置（フェーズ3の始点）
		const float endAngle    = kCamAngle + kMasterSwingDelta;
		const Vector3 swingEndPos = playerPos + Vector3(
			cosf(endAngle) * kMidRadius,
			kMasterMidHeight,
			sinf(endAngle) * kMidRadius
		);

		const Vector3 masterMidTarget = playerPos + Vector3(0.0f, kMasterMidTargetY, 0.0f);

		if (swingT < 1.0f)
		{
			// フェーズ2: 旋回（終了0.3秒前にシェイク）
			if (!masterShakeTriggered_ && swingElapsed >= kMasterSwingDuration - 0.6f)
			{
				resultCamera_->StartShake(app::camera::ShakeSize::Medium);
				masterShakeTriggered_ = true;
			}
			camData.position = playerPos + Vector3(
				cosf(swingAngle) * kMidRadius,
				kMasterMidHeight,
				sinf(swingAngle) * kMidRadius
			);
			camData.target = masterMidTarget;
		}
		else
		{
			// フェーズ3: 顔クローズアップ
			const float faceT = EaseOutCubic(
				min((swingElapsed - kMasterSwingDuration) / kFaceZoomDuration, 1.0f)
			);

			const Vector3 faceTarget = playerPos + Vector3(0.0f, kFaceHeight, 0.0f);
			const Vector3 faceCamPos = playerPos + Vector3(
				cosf(endAngle) * kMasterFaceCamRadius,
				kFaceHeight,
				sinf(endAngle) * kMasterFaceCamRadius
			);

			camData.position.Lerp(faceT, swingEndPos, faceCamPos);
			camData.target.Lerp(faceT, masterMidTarget, faceTarget);
		}
	}
	else if (debugRankIndex_ == 1) // ELITE: 後方静止 → 旋回＋顔ズーム
	{
		const float backAngle         = kCamAngle + kEliteSwingAngle;
		const Vector3 eliteBackPos    = playerPos + Vector3(
			cosf(backAngle) * kMidRadius,
			kMidHeight,
			sinf(backAngle) * kMidRadius
		);
		const Vector3 eliteBackTarget = playerPos + Vector3(0.0f, kEliteSwingTargetY, 0.0f);

		if (cameraTime_ < kEliteHoldDuration)
		{
			// フェーズ1: 後方ミディアムショットで静止
			camData.position = eliteBackPos;
			camData.target   = eliteBackTarget;
		}
		else
		{
			// フェーズ2: 後方から旋回しながら顔へズームイン
			const float t = EaseInOutCubic(
				min((cameraTime_ - kEliteHoldDuration) / kEliteSwingDuration, 1.0f)
			);

			const float swingAngle = kCamAngle + kEliteSwingAngle * (1.0f - t);
			const float radius     = kMidRadius + (kEliteCamRadius - kMidRadius) * t;
			const float height     = kMidHeight + (kFaceHeight + kEliteCamYOffset - kMidHeight) * t;
			const float targetY    = kEliteSwingTargetY + (kFaceHeight + kEliteCamYOffset - kEliteSwingTargetY) * t;

			camData.position = playerPos + Vector3(
				cosf(swingAngle) * radius,
				height,
				sinf(swingAngle) * radius
			);
			camData.target = playerPos + Vector3(0.0f, targetY, 0.0f);
		}
	}
	else
	{
		// 未定義ランク: ミディアムショットで停止
		camData.position = midPos;
		camData.target   = midTarget;
	}

	// 時間とともに注視点をずらして左右寄せ
	float framingOffset;
	if      (debugRankIndex_ == 0) framingOffset = kMasterFramingMaxOffset;
	else if (debugRankIndex_ == 1) framingOffset = kEliteFramingOffset;
	else                           framingOffset = kFramingMaxOffset;
	const float framingT = EaseOutCubic(min(cameraTime_ / kFramingDuration, 1.0f));
	camData.target.z -= framingOffset * framingT;

	resultCamera_->SetState(camData);
}


void ResultScene::Update()
{
	const float dt = g_gameTime->GetFrameDeltaTime();

	UpdateCameraWork(dt);
	app::camera::CameraManager::Get().Update(dt);
	app::camera::CameraManager::Get().SetScreenEffectFocusWorldPos(modelTransform_.position);

	if (stageModel_) {
		stageModel_->Update();
	}

	if (playerModel_) {
		// デバッグ: LB1 + 右/左 でランクアニメーションを切り替え
		if (g_pad[0]->IsPress(enButtonLB1)) {
			bool switched = false;
			if (g_pad[0]->IsTrigger(enButtonRight)) {
				debugRankIndex_ = (debugRankIndex_ + 1) % 3;
				switched = true;
			}
			else if (g_pad[0]->IsTrigger(enButtonLeft)) {
				debugRankIndex_ = (debugRankIndex_ + 2) % 3;
				switched = true;
			}

			if (switched) {
				playerModel_->PlayAnimation(debugRankIndex_, 0.0f);
				playerModel_->SetAnimationSpeed(1.0f);
				cameraTime_              = 0.0f;
				beginnerShakeTriggered_  = false;
				masterShakeTriggered_    = false;
				beginnerIdlePlaying_      = false;
				beginnerIdleSlowMode_     = false;
				beginnerIdlePhase_        = 0.0f;
				beginnerIdleTime_         = 0.0f;
				beginnerIdleTotalDuration_= 0.0f;
				eliteIdlePlaying_         = false;
				masterIdlePlaying_        = false;
				masterIdleSlowMode_       = false;
				masterIdlePhase_          = 0.0f;
				masterIdleTime_           = 0.0f;
				masterIdleTotalDuration_  = 0.0f;
				auto* resultMenu = dynamic_cast<app::ui::ResultMenu*>(layout_.GetMenu());
				if (resultMenu) resultMenu->Reset(debugRankIndex_);
			}

			// デバッグ: LB1 + 上/下 で数値を増減して全桁に反映
			auto* resultMenu = dynamic_cast<app::ui::ResultMenu*>(layout_.GetMenu());
			if (resultMenu) {
				if (g_pad[0]->IsTrigger(enButtonUp)) {
					if (debugTestNumber_ < 9999) debugTestNumber_++;
					resultMenu->DebugSetNumber(debugTestNumber_);
				}
				else if (g_pad[0]->IsTrigger(enButtonDown)) {
					if (debugTestNumber_ > 0) debugTestNumber_--;
					resultMenu->DebugSetNumber(debugTestNumber_);
				}
			}
		}

		// 各ランク: メインアニメ終了後にアイドルへ遷移
		if (debugRankIndex_ == 2 && !beginnerIdlePlaying_ && !playerModel_->IsPlayingAnimation()) {
			const auto& topKF          = animationClips_[3].GetTopBoneKeyFrameList();
			beginnerIdleTotalDuration_ = topKF.empty() ? 1.0f : topKF.back()->time;
			beginnerIdlePhase_         = 0.0f;
			beginnerIdleSlowMode_      = false;
			beginnerIdleTime_          = 0.0f;
			playerModel_->PlayAnimation(3, 0.7f);
			playerModel_->SetAnimationSpeed(0.0f);
			playerModel_->SetAnimationCurrentTime(beginnerIdleTime_);
			beginnerIdlePlaying_ = true;
		}
		if (debugRankIndex_ == 1 && !eliteIdlePlaying_ && !playerModel_->IsPlayingAnimation()) {
			playerModel_->PlayAnimation(4, 1.0f);
			eliteIdlePlaying_ = true;
		}
		if (debugRankIndex_ == 0 && !masterIdlePlaying_ && !playerModel_->IsPlayingAnimation()) {
			const auto& topKF        = animationClips_[5].GetTopBoneKeyFrameList();
			masterIdleTotalDuration_ = topKF.empty() ? 1.0f : topKF.back()->time;
			masterIdlePhase_         = 0.0f;
			masterIdleSlowMode_      = false;
			masterIdleTime_          = 0.0f;
			playerModel_->PlayAnimation(5, 0.7f); // 長めにブレンドして馴染ませる
			playerModel_->SetAnimationSpeed(0.0f);
			playerModel_->SetAnimationCurrentTime(masterIdleTime_);
			masterIdlePlaying_ = true;
		}

		// BEGINNER ping-pong（MASTERと同じcos波駆動）
		if (beginnerIdlePlaying_) {
			const float rate = beginnerIdleSlowMode_ ? kMasterIdleSlowPhaseRate : kMasterIdlePhaseRate;
			beginnerIdlePhase_ += dt * rate;
			if (!beginnerIdleSlowMode_ && beginnerIdlePhase_ >= kPingPongPI) {
				beginnerIdleSlowMode_ = true;
			}
			if (beginnerIdlePhase_ >= kPingPongTwoPI) {
				beginnerIdlePhase_ -= kPingPongTwoPI;
			}
			beginnerIdleTime_ = (1.0f - cosf(beginnerIdlePhase_)) * 0.5f * beginnerIdleTotalDuration_;
			playerModel_->SetAnimationCurrentTime(beginnerIdleTime_);
		}

		// MASTERピンポン再生: 順方向と逆方向を端で反転。２段階目（初の逆方向切替）以降は半速
		if (masterIdlePlaying_) {
			const float rate = masterIdleSlowMode_ ? kMasterIdleSlowPhaseRate : kMasterIdlePhaseRate;
			masterIdlePhase_ += dt * rate;
			// 順再生1回分(π)が終わったら2段階目へ
			if (!masterIdleSlowMode_ && masterIdlePhase_ >= kPingPongPI) {
				masterIdleSlowMode_ = true;
			}
			if (masterIdlePhase_ >= kPingPongTwoPI) {
				masterIdlePhase_ -= kPingPongTwoPI;
			}
			masterIdleTime_ = (1.0f - cosf(masterIdlePhase_)) * 0.5f * masterIdleTotalDuration_;
			playerModel_->SetAnimationCurrentTime(masterIdleTime_);
		}

		playerModel_->SetTRS(modelTransform_.position, modelTransform_.rotation, modelTransform_.scale);
		playerModel_->Update();
	}

	layout_.Update();
}


void ResultScene::Render(RenderContext& rc)
{
	if (stageModel_) {
		stageModel_->Draw(rc);
	}
	if (playerModel_) {
		playerModel_->Draw(rc);
	}
	layout_.Render(rc);
}


bool ResultScene::RequestScene(uint32_t& id, float& waitTime)
{
	auto* resultMenu = dynamic_cast<app::ui::ResultMenu*>(layout_.GetMenu());
	if (!resultMenu) return false;

	if (resultMenu->IsReturnTitleDecided()) {
		id = TitleScene::ID();
		waitTime = 1.0f;
		return true;
	}

	if (resultMenu->IsRetryDecided()) {
		id = BattleScene::ID(); 
		waitTime = 1.0f;
		return true;
	}

	if (resultMenu->IsExitDecided()) {
		PostQuitMessage(0);
		return false;
	}
	return false;
}
