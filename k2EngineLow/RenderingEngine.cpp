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
	}


	void RenderingEngine::InitShadowMap()
	{
		m_shadow.Init();
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
		//影の描画
		m_shadow.Render(rc, m_renderObjects);
		//PreRender2D(rc);
		//レンダリングターゲットをメインレンダリングターゲットに変更
		rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderingTarget);
		//レンダリングターゲットを設定
		rc.SetRenderTargetAndViewport(m_mainRenderingTarget);
		//レンダリングターゲットをクリア
		rc.ClearRenderTargetView(m_mainRenderingTarget);

		//モデルの描画
		ModelDraw(rc);
		//エフェクトの描画
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
			renderObj->OnRenderModel(rc);
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
			renderObj->OnRender2D(rc);
		}
	}
}