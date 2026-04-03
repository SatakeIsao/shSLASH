#include "stdafx.h"
#include "CameraSteering.h"
#include "actor/Actor.h"


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


			// 理想のカメラ位置を計算（ターゲットの後ろ・上）
			// ※簡易的にZ軸手前に引いていますが、本来はターゲットの向き(Rotation)も考慮して回転させます
			Vector3 targetPosition = targetCharacter_->transform.position;
			targetPosition.y += 50.0f;
			Vector3 position = targetCharacter_->transform.position + toVector_;
			
			nextData.position = position;
			nextData.target = targetPosition;

			// 右スティックで回転
			Vector3 rotationVector = Vector3(g_pad[0]->GetRStickXF(), g_pad[0]->GetRStickYF(), 0.0f);
			if (rotationVector.LengthSq() > MOVE_MIN_FLOAT) {
				rotationVector.x *= config_.rotationSpeedX;
				rotationVector.y *= config_.rotationSpeedY;
				// rotXでY軸回転
				Quaternion yRotation;
				yRotation.SetRotationY(-rotationVector.x);
				yRotation.Apply(toVector_);
				// rotYでXZ軸回転
				Quaternion xzRotation;
				xzRotation.SetRotation(g_camera3D->GetRight(), -rotationVector.y);
				xzRotation.Apply(toVector_);
			}
			nextData.position = targetCharacter_->transform.position + toVector_;
			data = nextData;
		}
	}
}