/*!
 * @brief	キャラクタコントローラー。
 */

#pragma once
#include "CapsuleCollider.h"
#include "RigidBody.h"


namespace nsK2EngineLow
{
	class CharacterController : public Noncopyable
	{
	private:
		/** キャラクター用なのでカプセルとしておく */
		CCapsuleCollider collider_;
		/** 物理空間の処理に必要 */
		RigidBody rigidBody_;

		/** 座標 */
		Vector3 position_;
		/** 前回の座標 */
		Vector3 prevPosition_;

		/** 垂直方向の速度 */
		float verticalVelocity_ = 0.0f;
		/** 重力加速度 */
		float gravity_ = 0.0f;
		/** 半径 */
		float radius_ = 0.0f;
		/** 高さ */
		float height_ = 0.0f;

		/** 初期化済みフラグ */
		bool isInited_ = false;
		/** ジャンプ中フラグ */
		bool isJump_ = false;
		/** 地面に接地しているフラグ */
		bool isOnGround_ = true;
		/** テレポートリクエストフラグ */
		bool isRequestTeleport_ = false;


	public:
		CharacterController();
		~CharacterController();

		void Init(float radius, float height, const Vector3& position);

		/*!
		* @brief	実行。
		* @details  テレポートリクエストがある場合は衝突判定を行わずに移動します。
		* @param[in] targetPosition	移動先の座標。
		* @param[in] deltaTime		経過時間。
		*/
		const Vector3& Execute(const Vector3& targetPosition, float deltaTime);

		/*!
		* @brief	テレポートをリクエストする。
		* @details  これを呼んだ次の Execute() で、衝突判定を行わずに targetPosition へ移動します。
		*/
		void RequestTeleport()
		{
			isRequestTeleport_ = true;
		}

		void SetGravity(const float gravity) { gravity_ = gravity; }

		const Vector3& GetPosition() const { return position_; }
		const Vector3& GetPrevPosition() const { return prevPosition_; }
		void SetPosition(const Vector3& position) { position_ = position; }
		float GetVerticalVelocity() const { return verticalVelocity_; }
		bool IsJump() const { return isJump_; }
		bool IsOnGround() const { return isOnGround_; }
		void Jump(float jumpPower);

		CCapsuleCollider* GetCollider() { return &collider_; }
		RigidBody* GetRigidBody() { return &rigidBody_; }
		void RemoveRigidBoby();

		void Bounce(const float power)
		{
			// 地面にいるかどうかの判定を無視して、強制的に上方向の速度を上書きする！
			verticalVelocity_ = power;
			isOnGround_ = false; // 地面から離れたことにする
		}
	};
}