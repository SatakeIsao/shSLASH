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
				if (it != uiAnimationMap_.end()) {
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
			//CircularGaugeRender circularGaugeRender_;


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
				//circularGaugeRender_.SetPivot(pivot);
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
		
			// カスタムシェーダー用
			void Initialize(SpriteInitData& initData)
			{
				spriteRender_.Init(initData);
			}

			void SetMulColor(const Vector4& color)
			{
				spriteRender_.SetMulColor(color);
			}
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

			float w_;
			float h_;



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
		// 数字スプライトシート表示（UVオフセット方式）
		// ============================================

		/** numberSprite.fx の b1 定数バッファ */
		struct NumberSpriteCB {
			float uvOffsetX; // digit / ATLAS_COLUMNS
			float uvScaleX;  // 1 / ATLAS_COLUMNS
			float uvOffsetY; // コンテンツY開始UV（余白カット）
			float uvScaleY;  // コンテンツY幅UV
		};

		/**
		 * numbers.DDS アトラスを UVオフセットで切り抜いて数字を表示するクラス
		 * アトラスは横一列に 0〜9 が並んでいることを想定
		 */
		class UINumberSprite : public UIBase
		{
		private:
			static constexpr int   ATLAS_COLUMNS       = 10;
			static constexpr float UV_DIGIT_WIDTH      = 1.0f / ATLAS_COLUMNS;
			// numbers.DDS のコンテンツ領域（ピクセル座標 592〜952 / 1536）
			static constexpr float CONTENT_UV_OFFSET_Y = 0.3854f;
			static constexpr float CONTENT_UV_SCALE_Y  = 0.2344f;
			// 位置補正（per-digit UV 抽出に切り替えたため全ゼロ）
			static constexpr float DIGIT_X_CORRECTION[10] = {
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			};

			std::vector<SpriteRender*>    renderList_;
			// Sprite が保持するポインタが安定するようにヒープ確保
			std::vector<NumberSpriteCB*>  cbList_;
			int    number_        = 0;
			int    requestNumber_ = 0;
			int    digit_         = 1;
			int    maxDigit_      = 1;
			bool   isZeroPadding_ = false;
			std::string atlasPath_;
			float  w_ = 0;
			float  h_ = 0;
			float  uvXBias_      = 0.0f;
			float  uvXScaleBias_ = 0.0f;
			float  uvYScaleBias_ = 0.0f;
			// JSON から上書き可能な per-digit UV X 抽出範囲（セル幅に対する割合 [0,1]）
			// offset + scale <= 1.0 を守ること（超えると隣の桁にはみ出る）
			float  digitUVOffset_[10] = { 0.45f, 0.20f, 0.12f, 0.13f, 0.04f, 0.14f, 0.14f, 0.06f, 0.13f, 0.13f };
			float  digitUVScale_[10]  = { 1.00f, 0.28f, 0.76f, 0.74f, 0.91f, 0.75f, 0.76f, 0.80f, 0.75f, 0.76f };


		public:
			UINumberSprite();
			~UINumberSprite();


		public:
			virtual void Update() override;
			virtual void Render(RenderContext& rc) override;


		public:
			/**
			 * @param atlasPath  numbers.DDS のパス
			 * @param digit      最大桁数
			 * @param number     初期値
			 * @param width      1桁の表示幅
			 * @param height     1桁の表示高さ
			 */
			void Initialize(const char* atlasPath, int digit, int number,
				float width, float height,
				const Vector3& position, const Vector3& scale, const Quaternion& rotation);

			void SetNumber(int number)          { requestNumber_ = number; }
			void SetZeroPadding(bool padding)   { isZeroPadding_ = padding; }
			void SetXBias(float bias)
			{
				uvXBias_ = bias;
				for (int i = 0; i < static_cast<int>(cbList_.size()); i++) {
					UpdateDigitUV(i);
				}
			}
			void SetScaleBias(float xScale, float yScale)
			{
				uvXScaleBias_ = xScale;
				uvYScaleBias_ = yScale;
				for (int i = 0; i < static_cast<int>(cbList_.size()); i++) {
					UpdateDigitUV(i);
					cbList_[i]->uvScaleY = CONTENT_UV_SCALE_Y + uvYScaleBias_;
				}
			}
			// JSON "uvOffsets"/"uvScales" から per-digit UV テーブルを設定する
			// nullptr を渡した側は現在の値を維持する
			void SetDigitUVTable(const float* offsets, const float* scales, int count = 10)
			{
				const int n = (count < 10) ? count : 10;
				if (offsets) { for (int i = 0; i < n; i++) digitUVOffset_[i] = offsets[i]; }
				if (scales)  { for (int i = 0; i < n; i++) digitUVScale_[i]  = scales[i];  }
				for (int i = 0; i < static_cast<int>(cbList_.size()); i++) {
					UpdateDigitUV(i);
				}
			}
			std::vector<SpriteRender*>& GetSpriteRenderList() { return renderList_; }

			void ForEach(const std::function<void(SpriteRender*)>& func)
			{
				for (auto* r : renderList_) func(r);
			}


		private:
			void AddDigitSprite();
			void UpdateDigitUV(int index);
			void UpdatePosition(int index);
			int  GetDigitAt(int digitPos);
			int  ComputeDigitCount();
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