#include "k2EngineLowPreCompile.h"
#include "RenderingEngine.h"

namespace nsK2EngineLow {
	RenderingEngine::RenderingEngine()
	{}

	RenderingEngine::~RenderingEngine()
	{}

	//登録処理
	void RenderingEngine::Init()
	{
		//フレームバッファーの横幅、高さを取得
		int frameBuffer_w = g_graphicsEngine->GetFrameBufferWidth();
		int frameBuffer_h = g_graphicsEngine->GetFrameBufferHeight();

		float clearColor[4] = { 0.5f,0.5f,0.5f,1.0f };

		//メインレンダリングターゲット
		m_mainRenderingTarget.Create(
			frameBuffer_w,	//テクスチャの幅
			frameBuffer_h,	//テクスチャの高さ
			1,											//Mipmapレベル
			1,											//テクスチャ配列のサイズ
			DXGI_FORMAT_R32G32B32A32_FLOAT,				//カラーバッファのフォーマット
			DXGI_FORMAT_D32_FLOAT,						//デプスステンシルバッファのフォーマット
			clearColor
		);

		//フレームバッファーにテクスチャを貼り付けるためのスプライトを初期化
		//初期化オブジェクトを作成
		SpriteInitData spriteInitData;
		spriteInitData.m_textures[0] = &m_mainRenderingTarget.GetRenderTargetTexture();
		spriteInitData.m_width = frameBuffer_w;
		spriteInitData.m_height = frameBuffer_h;
		spriteInitData.m_fxFilePath = "Assets/Shader/sprite.fx";

		m_copyToFrameBufferSprite.Init(spriteInitData);

		m_bloom.InitRenderTarget(m_mainRenderingTarget);

		//2D(フォントやスプライト)用の初期化
		Init2DSprite();

		//ブルームの初期化
		InitBloom();

		//シャドウのための初期化
		InitShadowMap();

		//G-Bufferの初期化（ディファードレンダリング用）
		InitGBuffer();
	}


	void RenderingEngine::InitShadowMap()
	{
		m_shadow.Init();
	}

	void RenderingEngine::InitGBuffer()
	{
		int w = g_graphicsEngine->GetFrameBufferWidth();
		int h = g_graphicsEngine->GetFrameBufferHeight();
		// alpha=0 でクリア → ジオメトリなし画素の検出に使う
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		// アルベドRT（深度バッファあり — G-Bufferパスで共有）
		m_albedoRT.Create(w, h, 1, 1,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_D32_FLOAT,
			clearColor);

		// 法線RT（深度バッファなし — アルベドRTの深度を共有）
		m_normalRT.Create(w, h, 1, 1,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_UNKNOWN,
			clearColor);

		// ワールド座標RT（深度バッファなし）
		m_worldPosRT.Create(w, h, 1, 1,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_UNKNOWN,
			clearColor);

		// ディファードライティング用スプライト（G-Bufferを読んでライティング計算）
		SpriteInitData spriteInitData;
		spriteInitData.m_textures[0] = &m_albedoRT.GetRenderTargetTexture();
		spriteInitData.m_textures[1] = &m_normalRT.GetRenderTargetTexture();
		spriteInitData.m_textures[2] = &m_worldPosRT.GetRenderTargetTexture();
		spriteInitData.m_textures[3] = &m_shadow.GetRenderTarget().GetRenderTargetTexture();
		spriteInitData.m_width  = w;
		spriteInitData.m_height = h;
		spriteInitData.m_fxFilePath                = "Assets/Shader/deferredLighting.fx";
		spriteInitData.m_expandConstantBuffer      = &g_sceneLight->GetLightData();
		spriteInitData.m_expandConstantBufferSize  = sizeof(g_sceneLight->GetLightData());
		spriteInitData.m_colorBufferFormat[0]      = m_mainRenderingTarget.GetColorBufferFormat();

		m_deferredLightingSprite.Init(spriteInitData);
	}

	void RenderingEngine::InitBloom()
	{
		m_bloom.InitLumi(m_mainRenderingTarget);
		m_bloom.InitGaussBlur();
		m_bloom.InitBoke(m_mainRenderingTarget);
	}

	void RenderingEngine::Init2DSprite()
	{
		float clearColor[4] = { 0.0f,0.0f,0.0f,0.0f };
		//2D用のターゲットの初期化
		m_2DRenderTarget.Create(
			m_mainRenderingTarget.GetWidth(),
			m_mainRenderingTarget.GetHeight(),
			1,
			1,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_UNKNOWN,
			clearColor
		);

		//最終合成用のスプライトを初期化する
		SpriteInitData spriteInitData;
		//テクスチャは2Dレンダーターゲット
		spriteInitData.m_textures[0] = &m_2DRenderTarget.GetRenderTargetTexture();
		//解像度はmainRenderTargetの幅と高さ
		spriteInitData.m_width = m_mainRenderingTarget.GetWidth();
		spriteInitData.m_height = m_mainRenderingTarget.GetHeight();
		//2D用シェーダーを使用する
		spriteInitData.m_fxFilePath = "Assets/Shader/sprite.fx";
		spriteInitData.m_vsEntryPointFunc = "VSMain";
		spriteInitData.m_psEntryPoinFunc = "PSMain";
		//上書き
		spriteInitData.m_alphaBlendMode = AlphaBlendMode_None;
		//レンダリングターゲットのフォーマット
		spriteInitData.m_colorBufferFormat[0] = m_mainRenderingTarget.GetColorBufferFormat();

		m_2DSprite.Init(spriteInitData);

		//テクスチャはメインレンダーターゲット
		spriteInitData.m_textures[0] = &m_mainRenderingTarget.GetRenderTargetTexture();

		//解像度は2Dレンダーターゲットの幅と高さ
		spriteInitData.m_width = m_2DRenderTarget.GetWidth();
		spriteInitData.m_height = m_2DRenderTarget.GetHeight();

		//レンダリングターゲットのフォーマット
		spriteInitData.m_colorBufferFormat[0] = m_2DRenderTarget.GetColorBufferFormat();

		m_mainSprite.Init(spriteInitData);
	}

	void RenderingEngine::Execute(RenderContext& rc)
	{
		// 1. フォワードパス：スカイキューブ等をメインRTに描画
		//    後段の discard により、ジオメトリなし画素にのみ残る
		rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderingTarget);
		rc.SetRenderTargetAndViewport(m_mainRenderingTarget);
		rc.ClearRenderTargetView(m_mainRenderingTarget);
		ModelDraw(rc);
		rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderingTarget);

		// 2. シャドウパス
		m_shadow.Render(rc, m_renderObjects);

		// シャドウマップを描画した今フレームのLVPをSceneLightに即座に反映
		g_sceneLight->GetLightData().mLVP = m_shadow.GetLigCameraViewProjection();

		// 3. G-Bufferパス（ディファードレンダリング）
		RenderTarget* gBufferRTs[] = { &m_albedoRT, &m_normalRT, &m_worldPosRT };
		rc.WaitUntilToPossibleSetRenderTargets(3, gBufferRTs);
		rc.SetRenderTargetsAndViewport(3, gBufferRTs);
		rc.ClearRenderTargetViews(3, gBufferRTs);
		GBufferDraw(rc);
		rc.WaitUntilFinishDrawingToRenderTargets(3, gBufferRTs);

		// 4. ディファードライティングパス（メインRTへ、クリアしない）
		//    ジオメトリ画素 → ライティング結果で上書き
		//    スカイ画素    → discard によりフォワードパスのスカイキューブが透過
		rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderingTarget);
		rc.SetRenderTargetAndViewport(m_mainRenderingTarget);
		m_deferredLightingSprite.Draw(rc);

		// 4. エフェクト描画（フォワードレンダリングとして合成）
		EffectEngine::GetInstance()->Draw();
		//レンダリングターゲットへの書き込み終了待ち
		rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderingTarget);

		//画像と文字の描画
		SpriteFontDraw(rc);

		/** ブルーム */
		// 輝度抽出
		m_bloom.RenderLumi(rc);
		//ボケ画像を生成
		m_bloom.RenderGauss(rc);
		////ボケ画像を加算合成
		rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderingTarget);
		//レンダリングターゲットを設定
		rc.SetRenderTargetAndViewport(m_mainRenderingTarget);
		//最終合成
		m_bloom.FinalSpriteDraw(rc);
		//レンダリングターゲットの書き込み終了待ち
		rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderingTarget);

		//メインレンダリングターゲットの絵をフレームバッファにコピー
		CopyMainRenderTargetToFrameBuffer(rc);

		Render2DSprite(rc);
		//登録されている描画オブジェクトをクリア
		m_renderObjects.clear();
	}

	void RenderingEngine::ModelDraw(RenderContext& rc)
	{
		for (auto& renderObj : m_renderObjects)
		{
			if (renderObj == nullptr) { continue; }
			renderObj->OnRenderModel(rc);
		}
	}

	void RenderingEngine::GBufferDraw(RenderContext& rc)
	{
		for (auto& renderObj : m_renderObjects)
		{
			if (renderObj == nullptr) { continue; }
			renderObj->OnRenderToGBuffer(rc);
		}
	}

	void RenderingEngine::CopyMainRenderTargetToFrameBuffer(RenderContext& rc)
	{
		//フレームバッファもセット
		rc.SetRenderTarget(
			g_graphicsEngine->GetCurrentFrameBuffuerRTV(),
			g_graphicsEngine->GetCurrentFrameBuffuerDSV()
		);
		rc.SetViewportAndScissor(g_graphicsEngine->GetFrameBufferViewport());
		//bloom.GetCopyToFrameBuffer().Draw(rc);
		m_copyToFrameBufferSprite.Draw(rc);
	}

	void RenderingEngine::SpriteFontDraw(RenderContext& rc)
	{
		//2D用のターゲットが使えるようになるまで待つ
		rc.WaitUntilToPossibleSetRenderTarget(m_2DRenderTarget);
		//ターゲットセット
		rc.SetRenderTargetAndViewport(m_2DRenderTarget);
		//ターゲットのクリア
		rc.ClearRenderTargetView(m_2DRenderTarget);

		m_mainSprite.Draw(rc);

		m_2DSprite.Draw(rc);

		//描画されるまで待つ
		rc.WaitUntilFinishDrawingToRenderTarget(m_2DRenderTarget);
		//ターゲットをメインに戻す
		rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderingTarget);
		rc.SetRenderTargetAndViewport(m_mainRenderingTarget);
		//メインレンダリングターゲットへの書き込み終了待ち
		rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderingTarget);
	}

	//2D描画処理
	void RenderingEngine::Render2DSprite(RenderContext& rc)
	{
		for (auto& renderObj : m_renderObjects)
		{
			if (renderObj == nullptr) { continue; }
			renderObj->OnRender2D(rc);
		}
	}
}