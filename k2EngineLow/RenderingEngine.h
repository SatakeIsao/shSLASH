#pragma once
#include "Shadow.h"
#include "Bloom.h"

namespace nsK2EngineLow {

	class Bloom;
	class SpriteRender;
	class FontRender;

	class RenderingEngine : public Noncopyable
	{
	public:
		RenderingEngine();
		~RenderingEngine();

		void Init();
		void InitShadowMap();
		void InitBloom();
		//void InitBloomLumi();
		//void InitBloomGauss();
		//void InitBloomBoke();
		void Init2DSprite();

		void AddRenderObject(IRenderer* renderObject)
		{
			m_renderObjects.emplace_back(renderObject);
		}

		/// <summary>
		/// スプライトレンダーをコンテナの後ろにくっつける
		/// </summary>
		/// <param name="spriteRender">スプライトレンダー</param>
		void AddSpriteRenderObject(IRenderer* spriteRender)
		{
			m_renderObjects.push_back(spriteRender);
		}
		/// <summary>
		/// フォントレンダーをコンテナの後ろにくっつける
		/// </summary>
		/// <param name="fontRender">フォントレンダー</param>
		void AddFontRenderObject(IRenderer* fontRender)
		{
			m_renderObjects.push_back(fontRender);
		}

		//void InitFinalSprite();
		//モデルの描画
		void ModelDraw(RenderContext& rc);
		//2Dモデルの描画
		void Render2DSprite(RenderContext& rc);
		//事前2Dモデルの描画
		//void PreRender2D(RenderContext& rc);
		//シャドウマップ描画処理
		//void RenderShadowDraw(RenderContext& rc);
		//実行
		void Execute(RenderContext& rc);

		void CopyMainRenderTargetToFrameBuffer(RenderContext& rc);

		void SpriteFontDraw(RenderContext& rc);
		////ライトビュースクリーンの設定
		//void SetLVP(Matrix mat)
		//{
		//	m_sceneLight.SetLVP(mat);
		//}
		//取得系の関数
		SceneLight& GetLightingCB()
		{
			return m_sceneLight;
		}


		RenderTarget& GetShadowMap()
		{
			return m_shadow.GetRenderTarget();
		}

		/*Camera& GetLigCamera()
		{
			return shadow.GetLigCamera();
		}*/

		//ライトカメラのビュープロジェクション行列
		const Matrix& GetLigCameraViewProjection()
		{
			return m_shadow.GetLigCameraViewProjection();
		}

		RenderTarget& GetBloom()
		{
			return m_bloom.GetRenderTarget();
		}

		Matrix& GetViewProjectionMatrix()
		{
			return m_viewProjectionMatrix;
		}

	private:
		SceneLight m_sceneLight;
		RenderTarget m_mainRenderingTarget;
		RenderTarget m_2DRenderTarget;
		Sprite m_mainSprite;
		Sprite m_2DSprite;
		//シャドウ用
		//RenderTarget m_shadowMapTarget;
		Camera m_lightCamera;
		Shadow m_shadow;
		//ブルーム用
		Bloom m_bloom;
		//RenderTarget* luminanceRenderTarget;
		Sprite m_copyToFrameBufferSprite;
		SpriteInitData m_spriteInitData;

		Matrix m_viewProjectionMatrix;
		std::vector<ModelRender*> m_modelRenderObject;
		std::vector<IRenderer* > m_renderObjects;	//描画オブジェクトのリスト
	};

}