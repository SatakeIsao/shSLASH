/**
 * Transform.h
 * 階層計算付きトランスフォームクラス
 */
#pragma once


namespace app
{
	namespace math
	{
		///** 必要な要素だけ */
		//class Transform
		//{
		//public:
		//	Vector3 position = Vector3::Zero;
		//	Vector3 scale = Vector3::One;
		//	Quaternion rotation = Quaternion::Identity;
		//};




		/** 階層計算付きトランスフォームクラス */
		class Transform : public Noncopyable
		{
		public:
			//自身のパラメータ
			Vector3 localPosition;
			Quaternion localRotation;
			Vector3 localScale;

			/** 親トランスフォームを考慮したパラメータ */
			Vector3 position;
			Quaternion rotation;
			Vector3 scale;


		private:
			Matrix rotationMatrix_;
			Matrix worldMatrix_;

			Transform* parent_;
			std::vector<Transform*> children_;


		public:
			Transform();
			~Transform();

			/** 更新処理 */
			void UpdateTransform();
			/** ワールド行列更新、UpdateTransformの方で呼ばれるので呼び出す必要なし */
			void UpdateWorldMatrix();

			/** 全ての子トランスフォームとの紐づけを外す */
			void Release();

			/** 特定の子トランスフォームとの紐づけを外す */
			void RemoveChild(Transform* t);
			/** すべてのトランスフォームとの紐づけを外す */
			void ClearChild();
			void AddChild(Transform* t)
			{
				// 同じのは追加しない
				if (FindChild(t)) {
					return;
				}
				children_.push_back(t);
			}
			bool FindChild(Transform* t)
			{
				for (auto* child : children_)
				{
					if (child == t) {
						return true;
					}
				}
				return false;
			}

			/** 親トランスフォームを設定 */
			void SetParent(Transform* p)
			{
				if (parent_) {
					return;
				}
				parent_ = p;
				parent_->AddChild(this);
			}

			/** 親トランスフォームがあるか */
			bool HasParent() const
			{
				return parent_ != nullptr;
			}
			/** 親を解除 */
			void ClearParent()
			{
				parent_ = nullptr;
			}

			/**
			 * 最初の子Transformを取得
			 * @return 子が存在すればそのポインタ、なければnullptr
			 */
			Transform* GetChild() const
			{
				if (!children_.empty()) {
					return children_.front();
				}
				return nullptr;
			}
		};
	}
}