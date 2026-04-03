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
		}


		void UIGauge::Render(RenderContext& rc)
		{
			spriteRender_.Draw(rc);
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
		}


		void UIText::Render(RenderContext& rc)
		{
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
				SpriteRender* spriteRender = new SpriteRender;
				spriteRender->Init(assetName, widht, height);
				spriteRender->SetPosition(position);
				spriteRender->SetScale(scale);
				spriteRender->SetRotation(rotation);
				renderList_.push_back(spriteRender);
				UpdateNumber(i + 1, number_);	// 桁なので＋１する
			}
		}


		void UIDigit::UpdateNumber(const int targetDigit, const int number)
		{
			// NOTE: targetDigitは1以上の値になっている
			K2_ASSERT(targetDigit >= 1, "桁指定が間違えています。\n");

			// いらない
			const int targetRenderIndex = targetDigit - 1;
			SpriteRender* nextRender = nullptr;
			// 次のやつをつくる
			if (targetRenderIndex < renderList_.size()) {
				nextRender = renderList_[targetRenderIndex];
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
				//こいつ入れるとHPバー反映されない
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