#include "stdafx.h"
#include "CameraController.h"
#include "CameraManager.h"


namespace app
{
	namespace camera
	{
		// ShakeSize ごとの { duration, intensity } プリセット（Small/Medium/Large の順）
		static constexpr struct { float duration; float intensity; }
		kShakePreset[] = {
			{ 0.15f, 1.5f },  // Small
			{ 0.25f, 4.0f },  // Medium
			{ 0.35f, 8.0f },  // Large
		};


		void GameCamera::StartShake(ShakeSize size)
		{
			if (!CameraManager::Get().IsShakeEnabled()) return;
			const auto& p   = kShakePreset[static_cast<int>(size)];
			shakeDuration_  = p.duration;
			shakeTimer_     = p.duration;
			shakeIntensity_ = p.intensity;
			shakeElapsed_   = 0.0f;
			isUpwardShake_  = false;
		}


		void GameCamera::StartShakeUpward(ShakeSize size)
		{
			if (!CameraManager::Get().IsShakeEnabled()) return;
			const auto& p   = kShakePreset[static_cast<int>(size)];
			shakeDuration_  = p.duration;
			shakeTimer_     = p.duration;
			shakeIntensity_ = p.intensity;
			shakeElapsed_   = 0.0f;
			isUpwardShake_  = true;
		}


		void GameCamera::Update()
		{
			if (shakeTimer_ <= 0.0f) return;

			const float dt = g_gameTime->GetFrameDeltaTime();
			shakeTimer_   -= dt;
			shakeElapsed_ += dt;

			const float decay = max(shakeTimer_ / shakeDuration_, 0.0f);
			const Vector3 up  = g_camera3D->GetUp();
			Vector3 offset;

			if (isUpwardShake_)
			{
				// -cos(t) で始まり：最初に下へ押し込まれ、次第に上へ掃き上がる
				const float offsetV = -cosf(shakeElapsed_ * 10.0f) * shakeIntensity_ * decay;
				offset = up * offsetV;
			}
			else
			{
				// 通常：横＋縦の鈍い揺れ
				const float offsetH = sinf(shakeElapsed_ * 10.0f) * shakeIntensity_ * decay;
				const float offsetV = cosf(shakeElapsed_ *  8.0f) * shakeIntensity_ * decay;
				const Vector3 right = g_camera3D->GetRight();
				offset = right * offsetH + up * offsetV;
			}

			data_.position += offset;
			data_.target   += offset;
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