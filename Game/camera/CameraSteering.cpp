#include "stdafx.h"
#include "CameraSteering.h"
#include "actor/Actor.h"
#include "CameraManager.h"

namespace
{
}


namespace app
{
	namespace camera
	{
		void CameraSteering::Update(CameraData& data, const float deltaTime)
		{
			if (targetCharacter_ == nullptr) {
				return;
			}
			CameraData nextData = data;


			// 仮想のカメラ位置を計算（ターゲットの後方など）
			// 簡易的な計算を前にいれていますが、本来はターゲットの回転(Rotation)も考慮して回転
			Vector3 targetPosition = targetCharacter_->transform.position;
			targetPosition.y += 50.0f;
			Vector3 position = targetCharacter_->transform.position + toVector_;
			
			nextData.position = position;
			nextData.target = targetPosition;

			float stickX = g_pad[0]->GetRStickXF();
			float stickY = g_pad[0]->GetRStickYF();

			// CameraManagerから設定を取得し、trueなら入力を反転
			if (CameraManager::Get().IsReverseX()) {
				stickX *= -1.0f;
			}
			if (CameraManager::Get().IsReverseY()) {
				stickY *= -1.0f;
			}

			// カメラ感度を適用
			float sensitivityMultiplier = CameraManager::Get().GetSensitivity() * 2.0f;
			stickX *= sensitivityMultiplier;
			stickY *= sensitivityMultiplier;

			// 回転を適用するため入力値(stickX, stickY)をVector3
			Vector3 rotationVector = Vector3(stickX, stickY, 0.0f);

			if (rotationVector.LengthSq() > MOVE_MIN_FLOAT) {
				rotationVector.x *= config_.rotationSpeedX;
				rotationVector.y *= config_.rotationSpeedY;
				// rotXはY軸回転
				Quaternion yRotation;
				yRotation.SetRotationY(-rotationVector.x);
				yRotation.Apply(toVector_);
				// rotYはXZ軸回転
				Vector3 nextToVector = toVector_;
				Quaternion xzRotation;
				xzRotation.SetRotation(g_camera3D->GetRight(), rotationVector.y);
				xzRotation.Apply(nextToVector);

				// toVector_のY軸で上下限界チェック
				// 各distance基準で sin(pitchMin/Max) * distance がY座標の限界
				const float len = toVector_.Length();
				const float yMin = len * sin(config_.pitchMin);
				const float yMax = len * sin(config_.pitchMax);
				if (nextToVector.y >= yMin && nextToVector.y <= yMax) {
					toVector_ = nextToVector;
				}
			}
			nextData.position = targetCharacter_->transform.position + toVector_;
			data = nextData;
		}
	}
}