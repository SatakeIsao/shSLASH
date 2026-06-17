#include "stdafx.h"
#include "CameraController.h"
#include "CameraManager.h"

namespace
{
	struct ShakePreset
	{
		/** 揺れ時間 */
		float duration;
		/** 揺れの強さ */
		float intensity;
	};

	/** 揺れ設定プリセット（低、中、強） */
	constexpr ShakePreset SHAKE_PRESETS[] = {
		{ 0.15f, 1.5f },
		{ 0.25f, 4.0f },
		{ 0.35f, 8.0f },
	};

	/** FOV計算用の微小値 */
	constexpr float FOV_EPSILION = 0.0001f;
	/** 各段階の溜めFOV */
	constexpr float CHARGE_FOVS[] = { 55.0f, 55.0f, 55.0f, 55.0f };
	/** FOV縮小開始までの時間 */
	constexpr float CHARGE_START_NARROW_TIME = 1.0f;
	/** FOV縮小にかかる時間 */
	constexpr float CHARGE_NARROW_TIME = 0.25f;
	/** FOVが元に戻る時間 */
	constexpr float CHARGE_RETURN_TIME = 0.12f;
	/** 待機判定のしきい時間 */
	constexpr float CHARGE_IDLE_THRESHOLD = 0.10f;
	/** 揺れ速度（主軸） */
	constexpr float SHAKE_WAVE_SPEED = 10.0f;
	/** 揺れ速度（縦方向） */
	constexpr float SHAKE_VERTICAL_WAVE_SPEED = 8.0f;
	/** 溜めFOVの最大インデックス */
	constexpr int	MAX_CHARGE_FOV_INDEX = 3;
}


namespace app
{
	namespace camera
	{
		void GameCamera::SetState(const CameraData& data)
		{
			data_ = data;

			const bool fovActive = fovIdleTimer_ < fovIdleThresh_
			                    || fabsf(fovCurrent_ - baseFov_) > FOV_EPSILION;
			if (!fovActive)
			{
				/** FOVエフェクトが発生していない間に、基準となるカメラの状態を同期 */
				baseFov_    = data.fov;
				fovCurrent_ = data.fov;
				fovTarget_  = data.fov;
			}
		}


		void GameCamera::StartShake(ShakeSize size)
		{
			if (!CameraManager::Get().IsShakeEnabled()) return;
			const auto& p   = SHAKE_PRESETS[static_cast<int>(size)];
			shakeDuration_  = p.duration;
			shakeTimer_     = p.duration;
			shakeIntensity_ = p.intensity;
			shakeElapsed_   = 0.0f;
			isUpwardShake_  = false;
		}


		void GameCamera::StartShakeUpward(ShakeSize size)
		{
			if (!CameraManager::Get().IsShakeEnabled()) return;
			const auto& p   = SHAKE_PRESETS[static_cast<int>(size)];
			shakeDuration_  = p.duration;
			shakeTimer_     = p.duration;
			shakeIntensity_ = p.intensity;
			shakeElapsed_   = 0.0f;
			isUpwardShake_  = true;
		}


		void GameCamera::OnAttackHit(float targetFovDeg, float fadeInDuration, float fadeOutDuration, float idleThreshold)
		{
			const float newTarget = Math::DegToRad(targetFovDeg);
			if (fabsf(newTarget - fovTarget_) > FOV_EPSILION)
			{
				fovFrom_        = fovCurrent_;
				fovNarrowTimer_ = 0.0f;
				fovTarget_      = newTarget;
			}
			fovNarrowTime_  = fadeInDuration;
			fovReturnTime_  = fadeOutDuration;
			fovIdleTimer_   = 0.0f;
			fovIdleThresh_  = idleThreshold;
			fovReturning_   = false;
		}


		void GameCamera::OnCharging(int chargingLevel)
		{
			// 溜め開始→オレンジまで 60°→55° の1段階のみ。オレンジ以降は変化なし
			const int idx = min(max(chargingLevel, 0), MAX_CHARGE_FOV_INDEX);
			const float newTarget = Math::DegToRad(CHARGE_FOVS[idx]);
			if (fabsf(newTarget - fovTarget_) > FOV_EPSILION)
			{
				fovFrom_        = fovCurrent_;
				fovNarrowTimer_ = 0.0f;
				fovTarget_      = newTarget;
				// 溜め開始→57°は長め、それ以降は短め
				fovNarrowTime_  = (chargingLevel == 0) ? CHARGE_START_NARROW_TIME : CHARGE_NARROW_TIME;
			}
			fovReturnTime_  = CHARGE_RETURN_TIME;
			fovIdleTimer_   = 0.0f;
			fovIdleThresh_  = CHARGE_IDLE_THRESHOLD;
			fovReturning_   = false;
		}


		void GameCamera::Update()
		{
			const bool hasShake = shakeTimer_ > 0.0f;
			const bool hasFov   = fovIdleTimer_ < fovIdleThresh_ || fabsf(fovCurrent_ - baseFov_) > FOV_EPSILION;
			if (!hasShake && !hasFov) return;

			const float dt = g_gameTime->GetFrameDeltaTime();

			if (hasShake)
			{
				shakeTimer_   -= dt;
				shakeElapsed_ += dt;

				const float decay = max(shakeTimer_ / shakeDuration_, 0.0f);
				const Vector3 up  = g_camera3D->GetUp();
				Vector3 offset;

				if (isUpwardShake_)
				{
					// -cos(t) で始まり：最初に下へ押し込まれ、次第に上へ掃き上がる
					const float offsetV = -cosf(shakeElapsed_ * SHAKE_WAVE_SPEED) * shakeIntensity_ * decay;
					offset = up * offsetV;
				}
				else
				{
					// 通常：横＋縦の鈍い揺れ
					const float offsetH = sinf(shakeElapsed_ * SHAKE_WAVE_SPEED) * shakeIntensity_ * decay;
					const float offsetV = cosf(shakeElapsed_ * SHAKE_VERTICAL_WAVE_SPEED) * shakeIntensity_ * decay;
					const Vector3 right = g_camera3D->GetRight();
					offset = right * offsetH + up * offsetV;
				}

				data_.position += offset;
				data_.target   += offset;
			}

			if (hasFov)
			{
				fovIdleTimer_ += dt;

				if (fovIdleTimer_ < fovIdleThresh_)
				{
					// 狭窄フェーズ：fovFrom_ → fovTarget_ を fovNarrowTime_ 秒で線形補間
					fovReturning_   = false;
					fovNarrowTimer_ = min(fovNarrowTimer_ + dt, fovNarrowTime_);
					const float t   = fovNarrowTime_ > 0.0f ? fovNarrowTimer_ / fovNarrowTime_ : 1.0f;
					fovCurrent_     = fovFrom_ + (fovTarget_ - fovFrom_) * t;
				}
				else
				{
					// 復帰フェーズ：fovCurrent_ → baseFov_ を fovReturnTime_ 秒で線形補間
					if (!fovReturning_)
					{
						fovReturning_   = true;
						fovFrom_        = fovCurrent_;
						fovNarrowTimer_ = 0.0f;
					}
					fovNarrowTimer_ = min(fovNarrowTimer_ + dt, fovReturnTime_);
					const float t   = fovReturnTime_ > 0.0f ? fovNarrowTimer_ / fovReturnTime_ : 1.0f;
					fovCurrent_     = fovFrom_ + (baseFov_ - fovFrom_) * t;
				}

				data_.fov = fovCurrent_;
			}
		}


#if defined(APP_DEBUG)
		void DebugCamera::OnEnter()
		{
			cameraData_ = CameraManager::Get().GetCurrentCameraData();
		}


		void DebugCamera::Update()
		{
			// fov変更
			if (g_pad[0]->IsPress(enButtonRB1)) {
				float value = g_pad[0]->GetLStickYF();
				cameraData_.fov += value * 0.05f;
				return;
			}
			// 左スティックで移動
			{
				Vector3 inputDirection;
				inputDirection.x = g_pad[0]->GetLStickXF();
				inputDirection.z = g_pad[0]->GetLStickYF();

				// カメラの前方向と右方向のベクトルを取得
				Vector3 forward = g_camera3D->GetForward();
				Vector3 right = g_camera3D->GetRight();

				// y方向には移動させない
				forward.y = 0.0f;
				right.y = 0.0f;

				// 左スティックの入力量を乗算
				right *= inputDirection.x;
				forward *= inputDirection.z;

				Vector3 direction = right + forward;
				direction.Normalize();
				// 移動速度を乗算
				direction.Scale(10.0f);

				// 平行移動
				cameraData_.position += direction;
				cameraData_.target += direction;
			}
			// 右スティックで回転
			{
				float rotX = g_pad[0]->GetRStickXF() * 0.05f;
				float rotY = g_pad[0]->GetRStickYF() * 0.05f;

				// rotXはY軸回転
				Quaternion yRotation;
				yRotation.SetRotationY(-rotX);
				Vector3 toVector = cameraData_.position - cameraData_.target;
				yRotation.Apply(toVector);
				// rotYはXZ軸回転
				Quaternion xzRotation;
				xzRotation.SetRotation(g_camera3D->GetRight(), -rotY);
				xzRotation.Apply(toVector);
				cameraData_.position = cameraData_.target + toVector;
			}
		}
#endif
	}
}
