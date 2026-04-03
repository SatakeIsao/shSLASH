/**
 * Transform.cpp
 * 階層計算付きトランスフォームクラス
 */
#include "stdafx.h"
#include "Transform.h"


namespace app
{
	namespace math
	{
		Transform::Transform()
			: position(Vector3::Zero)
			, localPosition(Vector3::Zero)
			, scale(Vector3::One)
			, localScale(Vector3::One)
			, rotation(Quaternion::Identity)
			, localRotation(Quaternion::Identity)
			, rotationMatrix_(Matrix::Identity)
			, worldMatrix_(Matrix::Identity)
			, parent_(nullptr)
		{
			children_.clear();
		}

		Transform::~Transform()
		{
			if (parent_) {
				parent_->RemoveChild(this);
			}
			Release();
		}

		void Transform::UpdateTransform()
		{

			if (parent_) {
				//座標計算
				Matrix localPos;
				localPos.MakeTranslation(localPosition);

				Matrix pos;
				pos.Multiply(localPos, parent_->worldMatrix_);

				//多分平行移動の部分を取ってるだけ
				position.x = pos.m[3][0];
				position.y = pos.m[3][1];
				position.z = pos.m[3][2];

				//スケール
				scale.x = localScale.x * parent_->scale.x;
				scale.y = localScale.y * parent_->scale.y;
				scale.z = localScale.z * parent_->scale.z;

				//回転
				rotation = parent_->rotation * localRotation;

			}
			else
			{
				//ローカルの値をそのままコピー
				position = localPosition;
				scale = localScale;
				rotation = localRotation;
			}

			//回転行列
			rotationMatrix_.MakeRotationFromQuaternion(rotation);
			//ワールド行列更新
			UpdateWorldMatrix();
		}

		void Transform::UpdateWorldMatrix()
		{
			Matrix scal, pos, world;
			scal.MakeScaling(scale);
			pos.MakeTranslation(position);

			world.Multiply(scal, rotationMatrix_);
			worldMatrix_.Multiply(world, pos);

			//子も更新
			for (Transform* child : children_)
			{
				child->UpdateTransform();
			}
		}


		void Transform::Release()
		{
			//イテレータ生成
			std::vector<Transform*>::iterator it = children_.begin();
			//vectorの終わりまで回す
			while (it != children_.end())
			{
				//子トランスフォームからの紐づけを外す
				(*it)->parent_ = nullptr;
				//子トランスフォームへの紐づけを外す
				children_.erase(it);
				// 対象がいないなら抜ける
				if (children_.size() == 0) {
					break;
				}
				//イテレータを進める
				++it;

			}
			children_.clear();
		}

		void Transform::RemoveChild(Transform* t)
		{
			auto it = std::find_if(children_.begin(), children_.end(), [&](Transform* child)
				{
					return child == t;
				});
			if (it == children_.end())
			{
				return;
			}
			(*it)->parent_ = nullptr;
			children_.erase(it);
		}


		void Transform::ClearChild()
		{
			children_.clear();
		}
	}
}