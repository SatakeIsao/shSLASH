#include "k2EngineLowPreCompile.h"
#include "SpriteRender.h"

namespace nsK2EngineLow {
	void SpriteRender::Init(const char* filePath, const float w, const float h, AlphaBlendMode alphaBlendMode)
	{
		SpriteInitData initData;
		// DDSファイルのファイルパスを指定
		initData.m_ddsFilePath[0] = filePath;
		// Sprite表示用のシェーダーのファイルパスを指定
		initData.m_fxFilePath = "Assets/Shader/sprite.fx";
		// スプライトの幅と高さを指定する
		initData.m_width = static_cast<UINT>(w);
		initData.m_height = static_cast<UINT>(h);
		initData.m_alphaBlendMode = alphaBlendMode;
		// Spriteの初期化オブジェクトを使用して、Spriteを初期化
		m_sprite.Init(initData);
	}

	void SpriteRender::Draw(RenderContext& rc)
	{
		if (g_renderingEngine->IsPreBlurMode() || g_renderingEngine->IsGlobalPreBlurMode())
			g_renderingEngine->AddPreBlurSpriteObject(this);
		else
			g_renderingEngine->AddSpriteRenderObject(this);
	}
}
