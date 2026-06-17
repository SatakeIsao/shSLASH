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

		// TBDR バッファを先に初期化（InitGBuffer がスプライトに参照させるため）
		InitTBDRBuffers();

		//G-Bufferの初期化（ディファードレンダリング用）
		InitGBuffer();

		// TBDR ディスクリプタヒープ初期化（m_worldPosRT が確定した後）
		InitTBDRDescriptorHeap();

		// 被写界深度初期化（m_worldPosRT が確定した後）
		InitDoF();

		// モーションブラー初期化
		InitMotionBlur();
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

		// TBDR: タイルインデックスリストとライトデータを expand SRV として登録
		// t20 = タイル別ライトインデックス, t21 = ライトデータ
		spriteInitData.m_expandShaderResoruceView[0] = &m_tileIndexUAV;
		spriteInitData.m_expandShaderResoruceView[1] = &m_tbdrLightBuffer;

		m_deferredLightingSprite.Init(spriteInitData);
	}

	void RenderingEngine::InitTBDRBuffers()
	{
		const int W = g_graphicsEngine->GetFrameBufferWidth();
		const int H = g_graphicsEngine->GetFrameBufferHeight();
		const int tilesX = (W + MAX_TBDR_TILE_WIDTH  - 1) / MAX_TBDR_TILE_WIDTH;
		const int tilesY = (H + MAX_TBDR_TILE_HEIGHT - 1) / MAX_TBDR_TILE_HEIGHT;
		const int numTiles = tilesX * tilesY;

		// ポイントライトデータ用 StructuredBuffer（CPU 毎フレーム更新）
		m_tbdrLightBuffer.Init(
			sizeof(TBDRPointLight),
			MAX_TBDR_POINT_LIGHT,
			nullptr);

		// タイル別ライトインデックス用 RWStructuredBuffer（コンピュートシェーダー出力）
		m_tileIndexUAV.Init(
			sizeof(uint32_t),
			MAX_TBDR_POINT_LIGHT * numTiles,
			nullptr);

		// カメラデータ CB（b0）
		m_tbdrCameraDataCB.Init(sizeof(TBDRCameraData), nullptr);

		// ライト数 CB（b1）
		TBDRParams params = { 0, { 0.f, 0.f, 0.f } };
		m_tbdrParamsCB.Init(sizeof(TBDRParams), &params);

		// コンピュートシェーダーロード
		m_lightCullingCS.LoadCS("Assets/Shader/lightCulling.fx", "CSMain");

		// ルートシグネチャ
		m_lightCullingRS.Init(
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP);

		// コンピュートパイプラインステート
		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = m_lightCullingRS.Get();
		psoDesc.CS             = CD3DX12_SHADER_BYTECODE(m_lightCullingCS.GetCompiledBlob());
		psoDesc.Flags          = D3D12_PIPELINE_STATE_FLAG_NONE;
		psoDesc.NodeMask       = 0;
		m_lightCullingPSO.Init(psoDesc);
	}

	void RenderingEngine::InitTBDRDescriptorHeap()
	{
		// コンピュートシェーダー用ディスクリプタヒープを構築
		// SRV: t0=worldPosRT, t1=lightBuffer
		// UAV: u0=tileIndexUAV
		// CBV: b0=cameraData, b1=params
		m_lightCullingDescHeap.RegistShaderResource(0, m_worldPosRT.GetRenderTargetTexture());
		m_lightCullingDescHeap.RegistShaderResource(1, m_tbdrLightBuffer);
		m_lightCullingDescHeap.RegistUnorderAccessResource(0, m_tileIndexUAV);
		m_lightCullingDescHeap.RegistConstantBuffer(0, m_tbdrCameraDataCB);
		m_lightCullingDescHeap.RegistConstantBuffer(1, m_tbdrParamsCB);
		m_lightCullingDescHeap.Commit();
	}

	void RenderingEngine::ExecuteTBDR(RenderContext& rc)
	{
		const int W = g_graphicsEngine->GetFrameBufferWidth();
		const int H = g_graphicsEngine->GetFrameBufferHeight();

		// --- ライトデータを GPU へコピー ---
		m_tbdrLightBuffer.Update(
			const_cast<TBDRPointLight*>(g_sceneLight->GetTBDRPointLightsData()));

		// --- カメラデータ CB を更新 ---
		TBDRCameraData camData;
		camData.mView = g_camera3D->GetViewMatrix();
		camData.mProj = g_camera3D->GetProjectionMatrix();
		camData.mProjInv.Inverse(camData.mProj);
		camData.screenParam = Vector4(
			g_camera3D->GetNear(),
			g_camera3D->GetFar(),
			static_cast<float>(W),
			static_cast<float>(H));
		m_tbdrCameraDataCB.CopyToVRAM(camData);

		// --- ライト数 CB を更新 ---
		TBDRParams params;
		params.numPointLight = g_sceneLight->GetNumTBDRPointLights();
		params._pad[0] = params._pad[1] = params._pad[2] = 0.f;
		m_tbdrParamsCB.CopyToVRAM(params);

		// --- コンピュートシェーダーをディスパッチ ---
		rc.SetComputeRootSignature(m_lightCullingRS);
		rc.SetComputeDescriptorHeap(m_lightCullingDescHeap);
		rc.SetPipelineState(m_lightCullingPSO);

		const int tilesX = (W + MAX_TBDR_TILE_WIDTH  - 1) / MAX_TBDR_TILE_WIDTH;
		const int tilesY = (H + MAX_TBDR_TILE_HEIGHT - 1) / MAX_TBDR_TILE_HEIGHT;
		rc.Dispatch(tilesX, tilesY, 1);

		// UAV → SRV 状態遷移（ディファードライティングパスで読み取れるようにする）
		rc.TransitionResourceState(
			m_tileIndexUAV.GetD3DResoruce(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	void RenderingEngine::InitBloom()
	{
		m_bloom.InitLumi(m_mainRenderingTarget);
		m_bloom.InitGaussBlur();
		m_bloom.InitBoke(m_mainRenderingTarget);
	}

	void RenderingEngine::InitDoF()
	{
		m_dof.Init(m_mainRenderingTarget, m_worldPosRT.GetRenderTargetTexture());
	}

	void RenderingEngine::InitMotionBlur()
	{
		m_motionBlur.Init(m_mainRenderingTarget);
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

		// 4a. TBDR ライトカリング（コンピュートシェーダー）
		ExecuteTBDR(rc);

		// 4b. ディファードライティングパス（メインRTへ、クリアしない）
		//    ジオメトリ画素 → ライティング結果で上書き
		//    スカイ画素    → discard によりフォワードパスのスカイキューブが透過
		rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderingTarget);
		rc.SetRenderTargetAndViewport(m_mainRenderingTarget);
		m_deferredLightingSprite.Draw(rc);

		// タイルインデックス UAV を元の状態に戻す（次フレームのコンピュートシェーダー用）
		rc.TransitionResourceState(
			m_tileIndexUAV.GetD3DResoruce(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

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

		// 被写界深度（有効時のみ実行）
		m_dof.Execute(rc);

		// モーションブラー（有効時のみ実行）
		m_motionBlur.Execute(rc);

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