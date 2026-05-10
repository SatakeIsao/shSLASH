#pragma once
#include "graphics/GaussianBlur.h"

namespace nsK2EngineLow {

	class Shadow
	{
	public:
		/// <summary>
		/// 初期化
		/// </summary>
		void Init();
		/// <summary>
		/// 描画
		/// </summary>
		/// <param name="rc">レンダリングコンテキスト</param>
		/// <param name="renderObjects"></param>
		void Render(
			RenderContext& rc,
			std::vector<IRenderer* >& renderObjects
		);

		const Vector3& GetLightPos() const
		{
			return m_lightCameraPos;
		}
		/// <summary>
		/// 影の描画
		/// </summary>
		/// <param name="rc"レンダリングコンテキスト></param>
		void SpriteShadowDraw(RenderContext& rc);

		RenderTarget& GetRenderTarget()
		{
			return m_shadowMap;
		}

		/*Camera& GetLigCamera()
		{
			return m_lightCamera;
		}*/

		const Matrix& GetLigCameraViewProjection()
		{
			return m_viewProjectionMatrix;
		}

		/// <summary>
		/// ぼかしたシャドウマップのテクスチャを取得
		/// </summary>
		Texture& GetShadowMapBlurTexture()
		{
			return m_shadowBlur.GetBokeTexture();
		}

	private:
		/// <summary>
		/// レンダーターゲットの初期化
		/// </summary>
		void InitRenderTarget();
		/// <summary>
		/// 影描画用のカメラの初期化
		/// </summary>
		void InitLightCamera();


	private:
		Camera m_lightCamera;
		// ライトカメラの座標
		Vector3 m_lightCameraPos;
		RenderTarget m_shadowMap;
		Sprite m_sprite;
		Matrix m_viewProjectionMatrix;
		// シャドウマップをぼかすためのガウシアンブラー
		GaussianBlur m_shadowBlur;
	};
}