#include "stdafx.h"
#include "CameraManager.h"


namespace app
{
	namespace camera
	{
		CameraManager* CameraManager::instance_ = nullptr;


        CameraManager::CameraManager()
        {
            // バネカメラの初期化
            springCamera_ = std::make_unique<SpringCamera>();
            springCamera_->Init(
                *g_camera3D,	// バネカメラの初期化を行うカメラを指定する
                1000.0f,		// カメラの移動速度の最大値
                true,			// カメラと地形との当たり判定を行うかどうかのフラグ。trueにすると当たり判定を行う
                5.0f			// カメラに設定される球のコリジョンの半径。第3引数がtrueの時に有効になる
            );
        }


        CameraManager::~CameraManager()
        {
        }


        void CameraManager::Setup(nsK2EngineLow::Camera* engineCamera)
        {
            engineCamera_ = engineCamera;
        }


        void CameraManager::ApplyScreenEffectSettings()
        {
            if (g_renderingEngine == nullptr) {
                return;
            }

            g_renderingEngine->SetDepthOfFieldEnabled(isScreenEffectActive_ && isDepthOfFieldEnabled_);
            g_renderingEngine->SetMotionBlurEnabled(isScreenEffectActive_ && isMotionBlurEnabled_);
        }


        void CameraManager::SetMotionBlurEnabled(bool enabled)
        {
            isMotionBlurEnabled_ = enabled;
            ApplyScreenEffectSettings();
        }


        void CameraManager::SetDepthOfFieldEnabled(bool enabled)
        {
            isDepthOfFieldEnabled_ = enabled;
            ApplyScreenEffectSettings();
        }


        void CameraManager::SetScreenEffectActive(bool active)
        {
            isScreenEffectActive_ = active;
            ApplyScreenEffectSettings();
        }


        void CameraManager::SetScreenEffectFocusWorldPos(const Vector3& position)
        {
            if (g_renderingEngine == nullptr) {
                return;
            }

            g_renderingEngine->SetDepthOfFieldFocusWorldPos(position);
        }


        void CameraManager::Register(const uint32_t nameHash, RefCameraController controller)
        {
            if (controllers_.find(nameHash) != controllers_.end()) {
				// 既に登録されている場合は無視
                return;
            }
            controllers_[nameHash] = controller;
        }


        void CameraManager::Unregister(const uint32_t nameHash)
        {
            if (controllers_.find(nameHash) == controllers_.end()) {
				// 登録されていない場合は無視
				return;
            }
            controllers_.erase(nameHash);
        }


        void CameraManager::SwitchCamera(const uint32_t nameHash, const float blendTime)
        {
            auto it = controllers_.find(nameHash);
            if (it == controllers_.end()) return;
			SwitchCamera(it->second, blendTime);
        }


        void CameraManager::SwitchCamera(RefCameraController controller, const float blendTime)
        {
#if defined(APP_DEBUG)
            prev_ = current_;
#endif
            if (!current_ || blendTime <= 0.0f) {
                // 即時切り替え：次フレームで正しい位置が揃ってからRefreshする
                current_ = controller;
                current_->OnEnter();
                next_ = nullptr;
                isBlending_ = false;
                pendingRefreshFrames_ = 2;
            } else {
                // ブレンド開始
                if (current_ != controller) {
                    next_ = controller;
                    next_->OnEnter();

                    // 現在のエンジンカメラの状態をブレンド開始地点として保存
                    blendStartData_.position = engineCamera_->GetPosition();
                    blendStartData_.target = engineCamera_->GetTarget();
                    blendStartData_.up = engineCamera_->GetUp();
                    blendStartData_.fov = engineCamera_->GetViewAngle();

                    blendDuration_ = blendTime;
                    blendTimer_ = 0.0f;
                    isBlending_ = true;
                }
            }
        }


        void CameraManager::Update(const float deltaTime)
        {
            if (engineCamera_ == nullptr) {
                return;
            }

            CameraData applyData;

            // 各コントローラーのUpdate
            if (current_) {
                current_->Update();
            }
            if (next_) {
                next_->Update();
            }

            // 補間の計算
            if (isBlending_ && next_) {
                blendTimer_ += deltaTime;
                const float t = blendTimer_ / blendDuration_;
                // 補間終了
                if (t >= 1.0f) {
                    isBlending_ = false;
                    current_ = next_;
                    next_ = nullptr;
                    applyData = current_->GetCameraData();
                } else {
                    // SmoothStepなどで滑らかにすると良い
                    // t = t * t * (3.0f - 2.0f * t); 
                    applyData = CameraData::Lerp(t, blendStartData_, next_->GetCameraData());
                }
            } else if (current_) {
                applyData = current_->GetCameraData();
            }

            if (isSpringCameraEnabled_) {
                // バネカメラ＋当たり判定あり
                springCamera_->SetTarget(applyData.target);
                springCamera_->SetPosition(applyData.position);
                if (pendingRefreshFrames_ > 0 && --pendingRefreshFrames_ == 0) {
                    springCamera_->Refresh();
                }
                springCamera_->Update();
            } else {
                // 直接反映（バネなし・当たり判定なし）
                engineCamera_->SetPosition(applyData.position);
                engineCamera_->SetTarget(applyData.target);
            }

            engineCamera_->SetUp(applyData.up);
            engineCamera_->SetViewAngle(applyData.fov);
            engineCamera_->SetNear(applyData.nearClip);
            engineCamera_->SetFar(applyData.farClip);
        }
	}
}
