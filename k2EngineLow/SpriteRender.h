#pragma once

namespace nsK2EngineLow
{

	class SpriteRender : public IRenderer
	{
	public:
		//初期化
		void Init(const char* filePath,								//ファイルパス
			const float w,											//画像の横幅
			const float h,											//画像の縦幅
			AlphaBlendMode alphaBlendMode = AlphaBlendMode_Trans);	//デフォルトは半透明合成

		//ユーザーが用意したデータで初期化
		void Init(SpriteInitData initData)
		{
			m_sprite.Init(initData);
		}
		
		//座標設定
		void SetPosition(const Vector3& pos)
		{
			m_position = pos;
		}

		//座標取得
		const Vector3& GetPosition()const
		{
			return m_position;
		}

		//大きさを設定
		void SetScale(const Vector3& scale)
		{
			m_scale = scale;
		}
		void SetScale(const float& scale)
		{
			m_scale.x = scale;
			m_scale.y = scale;
			m_scale.z = scale;
		}
		//大きさを乗算
		void AddScale(const float& scale)
		{
			m_scale *= scale;
		}
		void AddScale(const Vector3& scale)
		{
			m_scale.x *= scale.x;
			m_scale.y *= scale.y;
			m_scale.z *= scale.z;
		}

		//大きさを取得
		const Vector3& GetScale() const
		{
			return m_scale;
		}
		//回転を取得
		const Quaternion& GetRotation() const
		{
			return m_rotation;
		}

		//回転を設定
		void SetRotation(const Quaternion& rot)
		{
			m_rotation = rot;
		}

		//ピボットの設定
		void SetPivot(const Vector2& pivot)
		{
			m_pivot = pivot;
		}
		//ピボットを取得
		const Vector2& GetPivot()const
		{
			return m_pivot;
		}

		//乗算カラーを設定
		void SetMulColor(const Vector4& mulColor)
		{
			m_sprite.SetMulColor(mulColor);
		}
		//乗算カラーを取得
		const Vector4& GetMulColor()const
		{
			return m_sprite.GetMulColor();
		}

		//追加
		//void SetTexture(const std::string& filePath);

		//更新処理
		void Update()
		{
			m_sprite.Update(
				m_position,
				m_rotation,
				m_scale,
				m_pivot);
		}

		//描画処理
		void Draw(RenderContext& rc);

		void OnRender2D(RenderContext& rc) override
		{
			m_sprite.Draw(rc);
		}

	private:
	public:
	private:
		Sprite m_sprite;
		//Texture m_texture;
		Vector3 m_position = Vector3::Zero;
		Vector3 m_scale = Vector3::One;
		Quaternion m_rotation = Quaternion::Identity;
		Vector2	m_pivot = Sprite::DEFAULT_PIVOT;
	};

}