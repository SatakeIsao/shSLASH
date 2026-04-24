#include "k2EngineLowPreCompile.h"
#include "CircularGaugeRender.h"

namespace nsK2EngineLow
{
    void CircularGaugeRender::Init(const char* filePath, const float w, const float h, AlphaBlendMode alphaBlendMode)
    {
        SpriteInitData initData;
        initData.m_fxFilePath               = "Assets/Shader/CircularGauge.fx";
        initData.m_width                    = static_cast<UINT>(w);
        initData.m_height                   = static_cast<UINT>(h);
        initData.m_alphaBlendMode           = alphaBlendMode;
        initData.m_noTexture                = true;
        initData.m_expandConstantBuffer     = &m_gaugeCB;
        initData.m_expandConstantBufferSize = static_cast<int>(sizeof(m_gaugeCB));

        m_spriteRender.Init(initData);
    }
}
