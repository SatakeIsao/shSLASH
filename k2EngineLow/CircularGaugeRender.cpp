#include "k2EngineLowPreCompile.h"
#include "CircularGaugeRender.h"

namespace nsK2EngineLow
{
    void CircularGaugeRender::Init(
        const char* filePath,
        const float w,
        const float h,
        AlphaBlendMode alphaBlendMode)
    {
        SpriteInitData initData;
        /** 円形ゲージ専用シェーダーを使用する */
        initData.m_fxFilePath = "Assets/Shader/CircularGauge.fx";
        initData.m_width = static_cast<UINT>(w);
        initData.m_height = static_cast<UINT>(h);
        initData.m_alphaBlendMode = alphaBlendMode;
        /** テクスチャは不使用（シェーダーで数学的に描画） */
        initData.m_noTexture = true;
        /**
         * ゲージパラメータをb1レジスタの定数バッファとして登録
         * m_gaugeCBのポインタを渡すことで、毎フレームDraw時に自動的にGPUへ送信
         */
        initData.m_expandConstantBuffer = &m_gaugeCB;
        initData.m_expandConstantBufferSize = static_cast<int>(sizeof(m_gaugeCB));

        m_spriteRender.Init(initData);
    }
}
