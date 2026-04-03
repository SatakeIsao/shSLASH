/**
 * UIParts.h
 * UIのパーツ群
 */
#pragma once
#include "UIAnimation.h"


namespace app
{
	namespace ui
	{
		/** UI基底クラス */
		class UIBase : public Noncopyable
		{
		public:
			app::math::Transform transform;
			Vector4 color = Vector4::White;
			Vector2 pivot = Vector2(0.5f, 0.5f);

			bool isDraw = true;


		protected:
			std::unordered_map<uint32_t, std::unique_ptr<UIAnimationBase>> uiAnimationMap_;
			uint32_t key_;


		public:
			UIBase()
			{
				uiAnimationMap_.clear();
			}
			virtual ~UIBase()
			{
				uiAnimationMap_.clear();
			}

			virtual void Update() = 0;
			virtual void Render(RenderContext& rc) = 0;


		public:
			void SetKey(const uint32_t key)
			{
				key_ = key;
			}
			uint32_t GetKey() const { return key_; }



		public:
			void UpdateAnimation()
			{
				ForEachAnimation([](UIAnimationBase* animation)
					{
						animation->Update();
					});
			}
			void PlayAnimation()
			{
				ForEachAnimation([](UIAnimationBase* animation)
					{
						animation->Play();
					});
			}
			bool IsPlayAnimation()
			{
				auto it = std::find_if(uiAnimationMap_.begin(), uiAnimationMap_.end(), [&](const auto& animationPair)
					{
						auto* animation = animationPair.second.get();
						if (animation->IsPlay()) {
							return true;
						}
						return false;
					});
				return it != uiAnimationMap_.end();
			}
			void StopSpriteAnimation()
			{
				ForEachAnimation([](UIAnimationBase* animation)
					{
						animation->Stop();
					});
			}
			bool IsCompleted() const
			{
				// すべて再生済みか
				auto it = std::find_if(uiAnimationMap_.begin(), uiAnimationMap_.end(), [&](const auto& animationPair)
					{
						auto* animation = animationPair.second.get();
						return !animation->IsPlay();
					});
				return it != uiAnimationMap_.end();
			}


			void AddAnimation(const uint32_t key, std::unique_ptr<UIAnimationBase> animation)
			{
				animation->SetUI(this);
				uiAnimationMap_.emplace(key, std::move(animation));
			}


			void RemoveAnimation(const uint32_t key)
			{
				uiAnimationMap_.erase(key);
			}


			void ForEachAnimation(const std::function<void(UIAnimationBase*)>& func)
			{
				for (auto& animation : uiAnimationMap_) {
					func(animation.second.get());
				}
			}


			UIAnimationBase* FindAnimation(const uint32_t key)
			{
				auto it = uiAnimationMap_.find(key);
				if(it!= uiAnimationMap_.end()) {
					return it->second.get();
				}
				return nullptr;
			}
		};




		// ============================================
		// 画像を使うUI関連
		// ============================================


		class UIImage : public UIBase
		{
		protected:
			SpriteRender spriteRender_;


		public:
			UIImage();
			~UIImage();


		public:
			virtual void Update() override;
			virtual void Render(RenderContext& rc) override;

			void SetPivot(const Vector2& pivot)
			{
				this->pivot = pivot;
				spriteRender_.SetPivot(pivot);
			}
		};


		/**
		 * ゲージUI
		 */
		class UIGauge : public UIImage
		{
		public:
			UIGauge();
			~UIGauge();


		public:
			virtual void Update() override;
			virtual void Render(RenderContext& rc) override;

		public:
			void Initialize(const char* assetName, const float width, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation);
		};


		/**
		 * アイコンUI
		 */
		class UIIcon : public UIImage
		{
		public:
			UIIcon();
			~UIIcon();


		public:
			virtual void Update() override;
			virtual void Render(RenderContext& rc) override;


		public:
			void Initialize(const char* assetName, const float width, const float height);
		};




		// ============================================
		// 文字を使うUI関連
		// ============================================


		class UIText : public UIBase
		{
		protected:
			FontRender fontRender_;


		public:
			UIText();
			~UIText();


		public:
			virtual void Update() override;
			virtual void Render(RenderContext& rc) override;


			void SetPivot(const Vector2& pivot)
			{
				this->pivot = pivot;
				fontRender_.SetPivot(pivot);
			}


			void SetText(const wchar_t* text)
			{
				fontRender_.SetText(text);
			}
		};



		// ============================================
		// ボタンを使うUI関連
		// ============================================
		class UIButton : public UIImage
		{
		private:
			/** ボタンが押されたときの処理(外部から委譲される) */
			std::function<void()> delegate_;


		public:
			UIButton();
			~UIButton();


		public:
			virtual void Update() override;
			virtual void Render(RenderContext& rc) override;
		};



		// ============================================
		// UI桁表示(スコア表示などで使用)
		// ============================================
		class UIDigit : public UIBase
		{
		private:
			/** 画像表示機能の可変長配列 */
			std::vector<SpriteRender*> renderList_;
			/** 表示される数字 */
			int number_;
			int requestNumber_;
			int digit_;
			/** 初期化時の最大桁数（JSONで設定した桁数） */
			int maxDigit_;
			/** ゼロ埋めをするかどうかのフラグ */
			bool isZeroPadding_ = false;
			/** 数字表示に必要な画像が入った */
			std::string assetPath_;

			int w_;
			int h_;



		public:
			UIDigit();
			~UIDigit();


		public:
			virtual void Update() override;
			virtual void Render(RenderContext& rc) override;


		public:
			/**
			 * ・アセットの名前
			 * ・何桁かの情報（数）
			 * ・表示する数
			 * ・横
			 * ・高さ
			 * ・位置
			 * ・大きさ
			 * ・回転
			 */
			void Initialize(const char* assetPath, const int digit, const int number, const float widht, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation);

			/** 数字を設定 */
			void SetNumber(const int number) { requestNumber_ = number; }
			/** ゼロ埋めフラグの設定 */
			void SetZeroPadding(bool isPadding) { isZeroPadding_ = isPadding; }

			std::vector<SpriteRender*>& GetSpriteRenderList() { return renderList_; }

			void ForEach(const std::function<void(SpriteRender*)>& func)
			{
				for (auto* render : renderList_) {
					func(render);
				}
			}


		private:
			void UpdateNumber(const int targetDigit, const int number);
			void UpdatePosition(const int index);

			int ComputeDight();

			/** 対象の桁 */
			int GetDigit(int digit);
		};




		// ============================================
		// キャンバス
		// ============================================


		/**
		 * 絵を書くキャンバスのイメージ
		 * UIを作るときにこのクラスを作ってください
		 */
		class UICanvas : public UIBase
		{
			friend class UIBase;
			friend class UIImage;
			friend class UIGauge;
			friend class UIIcon;
			friend class UIText;
			friend class UIButton;


		private:
			/**
			 * NOTE: 各UI自体に親子関係持たせたいけど使わない可能性があるので、一旦ここだけにしてみる
			 *       実行順を担保したかったためvectorで処理
			 */
			std::vector<std::unique_ptr<UIBase>> uiList_;


		public:
			UICanvas();
			~UICanvas();


			void Update() override;
			void Render(RenderContext& rc) override;


		public:
			template <typename T>
			void CreateUI(const uint32_t key)
			{
				auto ui = std::make_unique<T>();
				ui->SetKey(key);
				ui->transform.SetParent(&transform);
				uiList_.push_back(std::move(ui));
			}

			void RemoveUI(const uint32_t key)
			{
				// TODO:本当はstd::find使いたい
				for (auto it = uiList_.begin(); it != uiList_.end(); it++) {
					if ((*it)->GetKey() == key) {
						uiList_.erase(it);
						break;
					}
				}
			}

			template <typename T>
			T* FindUI(const uint32_t key)
			{
				// TODO:本当はstd::find使いたい
				for (auto it = uiList_.begin(); it != uiList_.end(); it++) {
					if ((*it)->GetKey() == key) {
						return dynamic_cast<T*>(it->get());
					}
				}
				return nullptr;
			}
		};
	}
}