#pragma once

namespace nsK2EngineLow {
	/// <summary>
	/// レンダーベースの基底クラス。
	/// </summary>
	class IRenderer : public Noncopyable {
	public:
		// モデルの描画
		virtual void OnRenderModel(RenderContext& rc)
		{

		}

		// G-Bufferへの描画（ディファードレンダリング用）
		virtual void OnRenderToGBuffer(RenderContext& rc)
		{

		}

		// シャドウマップへの描画処理
		virtual void OnRenderShadowMap(RenderContext& rc, const Matrix& lvpMatrix)
		{

		}

		// 2D描画
		virtual void OnRender2D(RenderContext& rc)
		{

		}
	};

}