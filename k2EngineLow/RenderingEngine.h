#pragma once
#include "Shadow.h"
#include "Bloom.h"
#include "DepthOfField.h"
#include "MotionBlur.h"
#include "graphics/RootSignature.h"
#include "graphics/PipelineState.h"
#include "graphics/DescriptorHeap.h"
#include "graphics/StructuredBuffer.h"
#include "graphics/RWStructuredBuffer.h"
#include "graphics/ConstantBuffer.h"
#include "graphics/Shader.h"

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
		void InitDoF();
		void InitMotionBlur();
		//void InitBloomLumi();
		//void InitBloomGauss();
		//void InitBloomBoke();
		void Init2DSprite();
		void InitGBuffer();
		void InitTBDRBuffers();          // TBDR バッファ初期化（InitGBuffer より前に呼ぶ）
		void InitTBDRDescriptorHeap();   // TBDR ディスクリプタヒープ初期化（InitGBuffer より後に呼ぶ）
		void ExecuteTBDR(RenderContext& rc); // ライトカリング ディスパッチ

		void AddRenderObject(IRenderer* renderObject)
		{
			m_renderObjects.emplace_back(renderObject);
		}

		/// <summary>
		/// スプライトレンダラーをコンテナの後ろに追加する
		/// </summary>
		/// <param name="spriteRender">スプライトレンダラー</param>
		void AddSpriteRenderObject(IRenderer* spriteRender)
		{
			m_renderObjects.push_back(spriteRender);
		}
		/// <summary>
		/// フォントレンダラーをコンテナの後ろに追加する
		/// </summary>
		/// <param name="fontRender">フォントレンダラー</param>
		void AddFontRenderObject(IRenderer* fontRender)
		{
			m_renderObjects.push_back(fontRender);
		}

		//void InitFinalSprite();
		// モデルの描画
		void ModelDraw(RenderContext& rc);
		// G-Bufferへの描画（ディファードレンダリング用）
		void GBufferDraw(RenderContext& rc);
		// 2Dモデルの描画
		void Render2DSprite(RenderContext& rc);
		// 前景2Dモデルの描画
		//void PreRender2D(RenderContext& rc);
		// シャドウマップ描画処理
		//void RenderShadowDraw(RenderContext& rc);
		// 実行
		void Execute(RenderContext& rc);

		void CopyMainRenderTargetToFrameBuffer(RenderContext& rc);

		void SpriteFontDraw(RenderContext& rc);
		//// ライトビュープロジェクションの設定
		//void SetLVP(Matrix mat)
		//{
		//	m_sceneLight.SetLVP(mat);
		//}
		void SetTBDREnabled(bool enabled) { m_tbdrEnabled = enabled; }
		bool IsTBDREnabled() const { return m_tbdrEnabled; }

		// ゲッター系の関数
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

		// ライトカメラのビュープロジェクション行列
		const Matrix& GetLigCameraViewProjection()
		{
			return m_shadow.GetLigCameraViewProjection();
		}

		RenderTarget& GetBloom()
		{
			return m_bloom.GetRenderTarget();
		}

		void SetDepthOfFieldEnabled(bool enabled)
		{
			m_dof.SetEnabled(enabled);
		}

		void SetDepthOfFieldFocusWorldPos(const Vector3& position)
		{
			m_dof.SetPlayerWorldPos(position);
		}

		void SetMotionBlurEnabled(bool enabled)
		{
			m_motionBlur.SetEnabled(enabled);
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
		// シャドウ用
		//RenderTarget m_shadowMapTarget;
		Camera m_lightCamera;
		Shadow m_shadow;
		// ブルーム用
		Bloom m_bloom;
		// 被写界深度用
		DepthOfField m_dof;
		// モーションブラー用
		MotionBlur m_motionBlur;
		//RenderTarget* luminanceRenderTarget;
		Sprite m_copyToFrameBufferSprite;
		SpriteInitData m_spriteInitData;

		// G-Buffer用（ディファードレンダリング用）
		RenderTarget m_albedoRT;                // アルベド
		RenderTarget m_normalRT;                // 法線（ワールド空間法線）
		RenderTarget m_worldPosRT;              // ワールド座標
		Sprite       m_deferredLightingSprite;  // ディファードライティングスプライト

		Matrix m_viewProjectionMatrix;
		std::vector<ModelRender*> m_modelRenderObject;
		std::vector<IRenderer* > m_renderObjects;	// 描画オブジェクトのリスト

		// ---- TBDR（タイルベースディファードレンダリング）----
		static const int MAX_TBDR_POINT_LIGHT  = 1000;
		static const int MAX_TBDR_TILE_WIDTH   = 16;
		static const int MAX_TBDR_TILE_HEIGHT  = 16;

		// コンピュートシェーダー用カメラデータ（b0）
		struct alignas(16) TBDRCameraData
		{
			Matrix  mView;
			Matrix  mProj;
			Matrix  mProjInv;
			Vector4 screenParam; // near, far, width, height
		};

		// コンピュートシェーダー用ライト数パラメーター（b1）
		struct TBDRParams
		{
			int   numPointLight;
			float _pad[3];
		};

		bool               m_tbdrEnabled = false;		// TBDR ON/OFF フラグ
		StructuredBuffer   m_tbdrLightBuffer;			// ポイントライトデータ（TBDRPointLight 配列）
		RWStructuredBuffer m_tileIndexUAV;				// タイル別ライトインデックスリスト（UAV）
		ConstantBuffer     m_tbdrCameraDataCB;			// カメラデータ CB（b0）
		ConstantBuffer     m_tbdrParamsCB;				// ライト数 CB（b1）
		Shader             m_lightCullingCS;			// コンピュートシェーダー
		PipelineState      m_lightCullingPSO;			// コンピュートパイプラインステート
		RootSignature      m_lightCullingRS;			// ルートシグネチャ
		DescriptorHeap     m_lightCullingDescHeap;		// ディスクリプタヒープ
	};

}
