#pragma once
#include "graphics/GaussianBlur.h"


namespace nsK2EngineLow {
	/// <summary>
	/// ブルーム。
	/// </summary>
	class Bloom : public Noncopyable
	{
	public:
		
		
		/// <summary>
		/// ポストエフェクトを実行した結果となるテクスチャを取得
		/// </summary>
		/// <returns></returns>
		Texture& GetResultTexture()
		{
			return luminanceRenderTarget.GetRenderTargetTexture();
		}

		RenderTarget& GetRenderTarget()
		{
			return luminanceRenderTarget;
		}

		Sprite& GetCopyToFrameBuffer()
		{
			return copyToFrameBufferSprite;
		}
		

		/// <summary>
		/// 輝度抽出用のスプライトを初期化
		/// </summary>
		void InitLumi(RenderTarget& mainRenderTarget);
		/// <summary>
		/// 輝度抽出用のレンダリングターゲット
		/// </summary>
		/// <param name="rc"></param>
		void InitRenderTarget(RenderTarget& rc);
		/// <summary>
		/// ガウシアンブラーを初期化
		/// </summary>
		void InitGaussBlur();
		/// <summary>
		/// ボケ画像を加算合成するスプライトを初期化
		/// </summary>
		/// <param name="mainRenderTarget"></param>
		void InitBoke(RenderTarget& mainRenderTarget);
		/// <summary>
		/// 輝度抽出用のスプライトを描画
		/// </summary>
		/// <param name="rc"></param>
		void RenderLumi(RenderContext& rc);
		/// <summary>
		/// ガウシアンブラーを実行
		/// </summary>
		/// <param name="rc"></param>
		void RenderGauss(RenderContext& rc);
		/// <summary>
		/// ボケ画像をメインレンダリングターゲットに加算合成
		/// </summary>
		/// <param name="rc"></param>
		/// <param name="mainRenderTarget"></param>
		void RenderBoke(RenderContext& rc, RenderTarget& mainRenderTarget);

		void FinalSpriteDraw(RenderContext& rc)
		{
			finalSprite.Draw(rc);
		}

	private:
		//RenderTarget* mainRenderTarget;

		RenderTarget luminanceRenderTarget;	//輝度抽出用のレンダリングターゲット
		SpriteInitData luminanceSpriteInitData; //輝度抽出用のスプライトを初期化
		Sprite luminanceSprite;				

		GaussianBlur gaussianBlur[4];			//ガウシアンブラー

		SpriteInitData finalSpriteInitData; //ボケ画像を加算合成するスプライトを初期化
		Sprite finalSprite;					//加算合成用のスプライトを初期化
		//SpriteInitData spriteInitData;		//スプライトの初期化オブジェクト
		Sprite copyToFrameBufferSprite;		//フレームバッファにコピーするためのスプライトを初期化
	};
}


