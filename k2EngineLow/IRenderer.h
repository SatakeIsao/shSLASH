#pragma once
#pragma once

namespace nsK2EngineLow {
	/// <summary>
	/// レンダラーの基底クラス。
	/// </summary>
	class IRenderer : public Noncopyable {
	public:
		//モデルの描画
		virtual void OnRenderModel(RenderContext& rc)
		{

		}

		//シャドウマップへの描画処理
		virtual void OnRenderShadowMap(RenderContext& rc, const Matrix& lvpMatrix)
		{

		}

		//2D描画
		virtual void OnRender2D(RenderContext& rc)
		{

		}
	};

}

