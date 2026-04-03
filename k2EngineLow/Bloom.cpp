#include "k2EngineLowPreCompile.h"
#include "Bloom.h"

namespace nsK2EngineLow {

		void Bloom::InitRenderTarget(RenderTarget& renderTarget)
		{
			luminanceRenderTarget.Create(
				renderTarget.GetWidth(),
				renderTarget.GetHeight(),
				1,
				1,
				renderTarget.GetColorBufferFormat(),
				DXGI_FORMAT_D32_FLOAT
			);
		}


		void Bloom::InitLumi(RenderTarget& mainRenderTarget)
		{
			//輝度抽出用のシェーダー
			luminanceSpriteInitData.m_fxFilePath = "Assets/Shader/postEffect.fx";
			//頂点シェーダーのファイルパスを指定する
			luminanceSpriteInitData.m_vsEntryPointFunc = "VSMain";
			//ピクセルシェーダーのエントリーポイントを指定する
			luminanceSpriteInitData.m_psEntryPoinFunc = "PSSamplingLuminance";
			//スプライトの幅と高さは luminnceRenderTarget と同じ
			luminanceSpriteInitData.m_width = luminanceRenderTarget.GetWidth();
			luminanceSpriteInitData.m_height = luminanceRenderTarget.GetHeight();
			//テクスチャはメインレンダリングターゲットのカラーバッファ
			luminanceSpriteInitData.m_textures[0]
				= &mainRenderTarget.GetRenderTargetTexture();
			//書き込むレンダリングターゲットのフォーマットを指定する
			luminanceSpriteInitData.m_colorBufferFormat[0]
				= DXGI_FORMAT_R32G32B32A32_FLOAT;
			//作成した初期化情報をもとにスプライトを初期化する
			luminanceSprite.Init(luminanceSpriteInitData);
		}


		void Bloom::InitGaussBlur()
		{
			//gaussianBlur[0]は輝度テクスチャにガウスアンブラーをかける
			gaussianBlur[0].Init(&luminanceRenderTarget.GetRenderTargetTexture());
			//gaussianBlur[1]は gaussianBlur[0]のテクスチャにガウスアンブラーをかける
			gaussianBlur[1].Init(&gaussianBlur[0].GetBokeTexture());
			//gaussianBlur[2]は gaussianBlur[1]のテクスチャにガウスアンブラーをかける
			gaussianBlur[2].Init(&gaussianBlur[1].GetBokeTexture());
			//gaussianBlur[3]は gaussianBlur[2]のテクスチャにガウスアンブラーをかける
			gaussianBlur[3].Init(&gaussianBlur[2].GetBokeTexture());
		}

		void Bloom::InitBoke(RenderTarget& mainRenderTarget)
		{
			//ボケテクスチャを4枚指定
			finalSpriteInitData.m_textures[0] = &gaussianBlur[0].GetBokeTexture();
			finalSpriteInitData.m_textures[1] = &gaussianBlur[1].GetBokeTexture();
			finalSpriteInitData.m_textures[2] = &gaussianBlur[2].GetBokeTexture();
			finalSpriteInitData.m_textures[3] = &gaussianBlur[3].GetBokeTexture();

			//解像度は mainRenderTargetの幅と高さ
			finalSpriteInitData.m_width = mainRenderTarget.GetWidth();
			finalSpriteInitData.m_height = mainRenderTarget.GetHeight();

			//ぼかした画像を、通常の2Dとしてメインレンダリングターゲットの描画するので
			//2D用のシェーダーを使用する
			//finalSpriteInitData.m_fxFilePath = "Assets/shader/sample2D.fx";

			//ボケ画像を合成する必要があるので、2D用のシェーダーではなく、
			//専用シェーダーを指定
			finalSpriteInitData.m_fxFilePath = "Assets/Shader/postEffect.fx";
			finalSpriteInitData.m_psEntryPoinFunc = "PSBloomFinal";

			//ただし、加算合成で描画するので、アルファブレンディングモードを加算する
			finalSpriteInitData.m_alphaBlendMode = AlphaBlendMode_Add;

			//カラーバッファーのフォーマットは例によって、32ビット浮動小数点バッファー
			finalSpriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

			//初期化情報を元に加算合成用のスプライトを初期化
			finalSprite.Init(finalSpriteInitData);

			//テクスチャは mainRenderTarget のカラーバッファー
			//spriteInitData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();
			//spriteInitData.m_width = 1280;
			//spriteInitData.m_height = 720;

			//モノクロ用のシェーダーを指定する
			//spriteInitData.m_fxFilePath = "Assets/shader/sample2D.fx";

			//初期化オブジェクトを使って、スプライトを初期化
			//copyToFrameBufferSprite.Init(spriteInitData);
		}

		void Bloom::RenderLumi(RenderContext& rc)
		{
			//輝度抽出用のレンダリングターゲットに変更
			rc.WaitUntilToPossibleSetRenderTarget(luminanceRenderTarget);
			//レンダリングターゲットを設定
			rc.SetRenderTargetAndViewport(luminanceRenderTarget);
			//レンダリングターゲットをクリア
			rc.ClearRenderTargetView(luminanceRenderTarget);
			//輝度抽出を行う
			luminanceSprite.Draw(rc);

			//レンダリングターゲットへの書き込み終了待ち
			rc.WaitUntilFinishDrawingToRenderTarget(luminanceRenderTarget);
		}

		void Bloom::RenderGauss(RenderContext& rc)
		{
			gaussianBlur[0].ExecuteOnGPU(rc, 10);
			gaussianBlur[1].ExecuteOnGPU(rc, 10);
			gaussianBlur[2].ExecuteOnGPU(rc, 10);
			gaussianBlur[3].ExecuteOnGPU(rc, 10);
		}

		//void Bloom::RenderBoke(RenderContext& rc, RenderTarget& mainRenderTarget)
		//{
		//	//レンダリングターゲットとして利用できるまで待つ
		//	rc.WaitUntilToPossibleSetRenderTarget(mainRenderTarget);
		//	//レンダリングターゲットを設定
		//	rc.SetRenderTargetAndViewport(mainRenderTarget);
		//	//最終合成
		//	finalSprite.Draw(rc);
		//	//レンダリングターゲットへの書き込み終了待ち
		//	rc.WaitUntilFinishDrawingToRenderTarget(mainRenderTarget);
		//}


}
