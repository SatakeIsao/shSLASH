#pragma once

namespace nsK2EngineLow
{
	class FontRender:public IRenderer
	{
	public:
		static const int MAX_TEXT_SIZE = 256;
		~FontRender()
		{

		}

		//表示する文字を設定
		void SetText(const wchar_t* text)
		{
			swprintf_s(m_text, text);
		}

		//表示する文字を取得
		const wchar_t* GetText() const
		{
			return m_text;
		}

		//座標設定、ｚ値は0.0fで
		void SetPosition(float x, float y, float z)
		{
			SetPosition({ x,y,z });
		}
		//座標設定。ｚ値は0.0f
		void SetPosition(const Vector3& position)
		{
			m_position = position;
		}
		//座標を取得
		const Vector3& GetPosition() const
		{
			return m_position;
		}

		//大きさを設定
		void SetScale(const float scale)
		{
			m_scale = scale;
		}
		//大きさを取得
		const float GetScale() const
		{
			return m_scale;
		}

		//色を設定
		void SetColor(float r, float g, float b, float a)
		{
			SetColor({ r,g,b,a });
		}
		void SetColor(const Vector4& color)
		{
			m_color = color;
		}
		//色を取得
		const Vector4& GetColor() const
		{
			return m_color;
		}

		//回転を設定
		const float GetRotation() const
		{
			return m_rotation;
		}

		//ピボット設定
		// x = 0.5, y = 0.5 で画像の中心が基点
		// x = 0.0, y = 0.0 で画像の左下
		// x = 1.0, y = 1.0 で画像の右上
		void SetPivot(float x, float y)
		{
			SetPivot({ x,y });
		}
		void SetPivot(const Vector2& pivot)
		{
			m_pivot = pivot;
		}
		//ピボットの取得
		const Vector2& GetPivot() const
		{
			return m_pivot;
		}

		//影のパラメータを設定
		void SetShadowParam(bool isDrawShadow,	//影を描画する？
			float shadowOffset,					//影を描画する時のピクセルのオフセット量
			const Vector4& shadowColor)			//影の色
		{
			m_font.SetShadowParam(isDrawShadow, shadowOffset, shadowColor);
		}

		//描画処理
		void Draw(RenderContext& rc);
		void OnDraw(RenderContext& rc)
		{
			if (m_text == nullptr)
			{
				return;
			}

			OnRender2D(rc);
		}

	private:
		void OnRender2D(RenderContext& rc) override
		{
			m_font.Begin(rc);
			m_font.Draw(m_text, Vector2(m_position.x, m_position.y), m_color, m_rotation, m_scale, m_pivot);
			m_font.End(rc);
		}

	private:
		Vector3		 m_position = Vector3::Zero;		//座標
		float		 m_scale = 1.0f;					//文字の大きさ
		Vector4		 m_color = g_vec4White;				//文字の色、デフォルトで白
		float		 m_rotation = 0.0f;					//回転
		Vector2		 m_pivot = Sprite::DEFAULT_PIVOT;	//ピボット
		wchar_t		 m_text[MAX_TEXT_SIZE];				//文字
		Font		 m_font;							//フォント
	};
}


