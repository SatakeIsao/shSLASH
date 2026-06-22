/**
 * UIParts.cpp
 * UIのパーツ群
 */
#include "stdafx.h"
#include "UIParts.h"


namespace app
{
	namespace ui
	{
		// ============================================
		// 画像を使うUI関連
		// ============================================


		UIImage::UIImage()
		{
		}


		UIImage::~UIImage()
		{
		}


		void UIImage::Update()
		{
		}


		void UIImage::Render(RenderContext& rc)
		{
		}




		/************************************/


		UIGauge::UIGauge()
		{
		}


		UIGauge::~UIGauge()
		{
		}


		void UIGauge::Update()
		{
			transform.UpdateTransform();
			spriteRender_.SetPosition(transform.position);
			spriteRender_.SetScale(transform.scale);
			spriteRender_.SetRotation(transform.rotation);
			spriteRender_.Update();

			//circularGaugeRender_.SetPosition(transform.position);
			//circularGaugeRender_.SetScale(transform.scale);
			//circularGaugeRender_.SetRotation(transform.rotation);
			//circularGaugeRender_.Update();
		}


		void UIGauge::Render(RenderContext& rc)
		{
			spriteRender_.Draw(rc);
			//circularGaugeRender_.Draw(rc);
		}


		void UIGauge::Initialize(const char* assetName, const float width, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation)
		{
			transform.localPosition = position;
			transform.localScale = scale;
			transform.localRotation = rotation;

			spriteRender_.Init(assetName, width, height);
			spriteRender_.SetPosition(position);
			spriteRender_.SetScale(scale);
			spriteRender_.SetRotation(rotation);
			spriteRender_.Update();

			//circularGaugeRender_.Init(assetName, width, height);
			//circularGaugeRender_.SetPosition(position);
			//circularGaugeRender_.SetScale(scale);
			//circularGaugeRender_.SetRotation(rotation);
			//circularGaugeRender_.Update();
		}




		/************************************/


		UIIcon::UIIcon()
		{
		}


		UIIcon::~UIIcon()
		{
		}


		void UIIcon::Update()
		{
			UpdateAnimation();
			
			spriteRender_.SetMulColor(color);
			transform.UpdateTransform();
			spriteRender_.SetPosition(transform.position);
			spriteRender_.SetScale(transform.scale);
			spriteRender_.SetRotation(transform.rotation);
			spriteRender_.Update();
		}


		void UIIcon::Render(RenderContext& rc)
		{
			if (isDraw) {
				spriteRender_.Draw(rc);
			}
		}


		void UIIcon::Initialize(const char* assetName, const float width, const float height)
		{
			spriteRender_.Init(assetName, width, height);
		}




		/********************************/


		UIText::UIText()
		{
		}


		UIText::~UIText()
		{
		}


		void UIText::Update()
		{
			UpdateAnimation();

			transform.UpdateTransform();
			fontRender_.SetPosition(transform.position);
			fontRender_.SetScale(transform.scale.x);
			fontRender_.SetColor(color);
			fontRender_.SetPivot(pivot);
		}


		void UIText::Render(RenderContext& rc)
		{
			if (!isDraw) return;
			fontRender_.Draw(rc);
		}




		/********************************/


		UIButton::UIButton()
		{
		}


		UIButton::~UIButton()
		{
		}


		void UIButton::Update()
		{
		}


		void UIButton::Render(RenderContext& rc)
		{
		}




		/********************************/


		UIDigit::UIDigit()
		{
		}


		UIDigit::~UIDigit()
		{
		}


		void UIDigit::Update()
		{
			if (number_ != requestNumber_) {
				number_ = requestNumber_;
				
				//ゼロ埋めフラグがONなら最大桁数を維持、OFFなら桁数を計算
				if (isZeroPadding_) {
					digit_ = maxDigit_;
				}
				else {
					digit_ = ComputeDight();
				}

				//不要な桁を削除
				while (renderList_.size() > digit_) {
					delete renderList_.back();
					renderList_.pop_back();
				}

				for (int i = 0; i < digit_; ++i) {
					UpdateNumber(i + 1, number_);
				}
			}

			UpdateAnimation();

			transform.UpdateTransform();
			for (int i = 0; i < renderList_.size(); ++i)
			{
				auto* spriteRender = renderList_[i];
				UpdatePosition(i);
				spriteRender->SetScale(transform.scale);
				spriteRender->SetRotation(transform.rotation);
				spriteRender->SetMulColor(color);
				spriteRender->Update();
			}

		}


		void UIDigit::Render(RenderContext& rc)
		{
			if (!isDraw) {
				return;
			}

			for (SpriteRender* spriteRender : renderList_)
			{
				spriteRender->Draw(rc);
			}
		}


		void UIDigit::Initialize(const char* assetName, const int digit, const int number, const float widht, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation)
		{
			assetPath_ = assetName;
			digit_ = digit;
			maxDigit_ = digit;
			number_ = number;
			w_ = widht;
			h_ = height;

			transform.localPosition = position;
			transform.localScale = scale;
			transform.localRotation = rotation;

			for (int i = 0; i < digit; i++)
			{
				for (int i = 0; i < digit; i++)
				{
					UpdateNumber(i + 1, number_); // 桁なので＋１する
					auto* spriteRender = renderList_[i];
					spriteRender->SetPosition(position);
					spriteRender->SetScale(scale);
					spriteRender->SetRotation(rotation);
				}
			}
		}


		void UIDigit::UpdateNumber(const int targetDigit, const int number)
		{
			{
				// NOTE: targetDigitは1以上の値になっている
				K2_ASSERT(targetDigit >= 1, "桁指定が間違えています。\n");
				// いらない
				const int targetRenderIndex = targetDigit - 1;
				SpriteRender* nextRender = nullptr;
				// 次のやつをつくる
				if (targetRenderIndex < renderList_.size()) {
					delete renderList_[targetRenderIndex];
					renderList_[targetRenderIndex] = nullptr;
					nextRender = new SpriteRender();
					renderList_[targetRenderIndex] = nextRender;
				}
				else {
					nextRender = new SpriteRender();
					renderList_.push_back(nextRender);
				}
				// 対象の桁の数字
				const int targetDigitNumber = GetDigit(targetDigit);
				std::string assetNname = assetPath_ + "/0.dds";
				assetNname[assetNname.size() - 5] = '0' + targetDigitNumber;
				nextRender->Init(assetNname.c_str(), w_, h_);
			}
		}


		void UIDigit::UpdatePosition(const int index)
		{
			SpriteRender* render = renderList_[index];
			Vector3 position = transform.position;
			position.x -= w_ * index;
			render->SetPosition(position);
		}

		int UIDigit::ComputeDight()
		{
			int n = number_;
			if (n == 0) return 1;
			int count = 0;
			n = std::abs(n);
			while (n > 0) {
				n /= 10;
				count++;
			}
			return count;
		}


		int UIDigit::GetDigit(int digit)
		{
			// NOTE: targetDigitは1以上の値になっている
			K2_ASSERT(digit >= 1, "桁指定が間違えています。\n");
			digit -= 1;
			int divisor = static_cast<int>(pow(10, digit));
			return (number_ / divisor) % 10;
		}




		/************************************/
		// UINumberSprite

		constexpr float UINumberSprite::DIGIT_X_CORRECTION[10];

		UINumberSprite::UINumberSprite()
		{
		}


		UINumberSprite::~UINumberSprite()
		{
			for (auto* r  : renderList_) delete r;
			for (auto* cb : cbList_)     delete cb;
			renderList_.clear();
			cbList_.clear();
		}


		void UINumberSprite::Initialize(
			const char* atlasPath, int digit, int number,
			float width, float height,
			const Vector3& position, const Vector3& scale, const Quaternion& rotation)
		{
			atlasPath_ = atlasPath;
			digit_     = digit;
			maxDigit_  = digit;
			number_    = number;
			requestNumber_ = number;
			w_ = width;
			h_ = height;

			transform.localPosition = position;
			transform.localScale    = scale;
			transform.localRotation = rotation;

			for (int i = 0; i < digit; i++) {
				AddDigitSprite();
				UpdateDigitUV(i);
				renderList_[i]->SetPosition(position);
				renderList_[i]->SetScale(scale);
				renderList_[i]->SetRotation(rotation);
				renderList_[i]->Update();
			}
		}


		void UINumberSprite::Update()
		{
			if (number_ != requestNumber_) {
				number_ = requestNumber_;

				digit_ = isZeroPadding_ ? maxDigit_ : ComputeDigitCount();

				// 余剰スプライトを削除
				while (static_cast<int>(renderList_.size()) > digit_) {
					delete renderList_.back(); renderList_.pop_back();
					delete cbList_.back();     cbList_.pop_back();
				}
				// 不足スプライトを追加
				while (static_cast<int>(renderList_.size()) < digit_) {
					AddDigitSprite();
				}

				// 全桁の UV を更新（CopyToVRAM のみ・再初期化なし）
				for (int i = 0; i < digit_; i++) {
					UpdateDigitUV(i);
				}
			}

			UpdateAnimation();
			transform.UpdateTransform();

			for (int i = 0; i < static_cast<int>(renderList_.size()); i++) {
				UpdatePosition(i);
				renderList_[i]->SetScale(transform.scale);
				renderList_[i]->SetRotation(transform.rotation);
				renderList_[i]->SetMulColor(color);
				renderList_[i]->Update();
			}
		}


		void UINumberSprite::Render(RenderContext& rc)
		{
			if (!isDraw) return;
			for (auto* r : renderList_) r->Draw(rc);
		}


		void UINumberSprite::AddDigitSprite()
		{
			// ヒープ確保で安定したアドレスを持たせる
			// Sprite::Init が m_userExpandConstantBufferCPU にこのポインタを保存し、
			// 毎フレームの Draw で CopyToVRAM するため、アドレスが変わると壊れる
			auto* cb = new NumberSpriteCB();
			cb->uvOffsetX = 0.0f;
			cb->uvScaleX  = UV_DIGIT_WIDTH     + uvXScaleBias_;
			cb->uvOffsetY = CONTENT_UV_OFFSET_Y;
			cb->uvScaleY  = CONTENT_UV_SCALE_Y + uvYScaleBias_;

			SpriteInitData initData;
			initData.m_ddsFilePath[0]          = atlasPath_.c_str();
			initData.m_fxFilePath              = "Assets/Shader/numberSprite.fx";
			initData.m_width                   = static_cast<UINT>(w_);
			initData.m_height                  = static_cast<UINT>(h_);
			initData.m_expandConstantBuffer     = cb;
			initData.m_expandConstantBufferSize = sizeof(NumberSpriteCB);
			initData.m_alphaBlendMode           = AlphaBlendMode_Trans;

			auto* render = new SpriteRender();
			render->Init(initData);

			renderList_.push_back(render);
			cbList_.push_back(cb);
		}


		void UINumberSprite::UpdateDigitUV(int index)
		{
			const int dv = GetDigitAt(index + 1);
			cbList_[index]->uvOffsetX = (dv + digitUVOffset_[dv]) * UV_DIGIT_WIDTH + uvXBias_;
			cbList_[index]->uvScaleX  = digitUVScale_[dv] * UV_DIGIT_WIDTH + uvXScaleBias_;
		}


		void UINumberSprite::UpdatePosition(int index)
		{
			Vector3 pos = transform.position;
			pos.x -= w_ * index;

			// 各桁キャラクターのセル内重心補正（"0"の左余白が大きいなど）
			const int dv = static_cast<int>(cbList_[index]->uvOffsetX * ATLAS_COLUMNS + 0.5f);
			if (dv >= 0 && dv < 10) {
				pos.x += w_ * DIGIT_X_CORRECTION[dv];
			}

			renderList_[index]->SetPosition(pos);
		}


		int UINumberSprite::GetDigitAt(int digitPos)
		{
			// digitPos 1 = 1の位, 2 = 10の位, ...
			const int divisor = static_cast<int>(std::pow(10, digitPos - 1));
			return (std::abs(number_) / divisor) % 10;
		}


		int UINumberSprite::ComputeDigitCount()
		{
			int n = std::abs(number_);
			if (n == 0) return 1;
			int count = 0;
			while (n > 0) { n /= 10; count++; }
			return count;
		}


		/************************************/


		UICanvas::UICanvas()
		{
			uiList_.clear();
		}


		UICanvas::~UICanvas()
		{
			uiList_.clear();
		}


		void UICanvas::Update()
		{
			/** デバッグテスト */
			UpdateAnimation();

			transform.UpdateTransform();

			for (auto& ui : uiList_) {
				/** デバッグテスト */
				//ui.get()->color.w = this->color.w;

				ui.get()->Update();
			}
		}


		void UICanvas::Render(RenderContext& rc)
		{
			for (auto& ui : uiList_) {
				ui.get()->Render(rc);
			}
		}
	}
}