#include "k2EngineLowPreCompile.h"
#include "MotionBlur.h"

namespace
{
    /** ボケ効果が切り替わる速度 */
    constexpr float FADE_SPEED = 3.0f;
    /** ブラー適用量 */
    constexpr float BLUR_AMOUNT = 0.025f;
    /** ブラー中心X座標 */
    constexpr float BLUR_CENTER_X = 0.5f;
    /** ブラー中心Y座標 */
    constexpr float BLUR_CENTER_Y = 0.5f;
    /** ブラー効果の最小閾値 */
    constexpr float MIN_STRENGTH = 0.001f;
}

namespace nsK2EngineLow {

    void MotionBlur::Init(RenderTarget& mainRT)
    {
        m_mainRT = &mainRT;

        const int w = mainRT.GetWidth();
        const int h = mainRT.GetHeight();
        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

        m_tempRT.Create(w, h, 1, 1,
            mainRT.GetColorBufferFormat(),
            DXGI_FORMAT_UNKNOWN,
            clearColor);

        // メインRTをテンポラリRTへコピーするスプライト
        {
            SpriteInitData data;
            data.m_textures[0]          = &mainRT.GetRenderTargetTexture();
            data.m_width                = w;
            data.m_height               = h;
            data.m_fxFilePath           = "Assets/Shader/sprite.fx";
            data.m_colorBufferFormat[0] = m_tempRT.GetColorBufferFormat();
            data.m_alphaBlendMode       = AlphaBlendMode_None;
            m_copySprite.Init(data);
        }

        // テンポラリRTを読んでラジアルブラーをメインRTへ合成するスプライト
        {
            m_cbData.strength   = 0.0f;
            m_cbData.blurAmount = BLUR_AMOUNT;
            m_cbData.center     = Vector2(BLUR_CENTER_X, BLUR_CENTER_Y);

            SpriteInitData data;
            data.m_textures[0]              = &m_tempRT.GetRenderTargetTexture();
            data.m_width                    = w;
            data.m_height                   = h;
            data.m_fxFilePath               = "Assets/Shader/motionBlur.fx";
            data.m_colorBufferFormat[0]     = mainRT.GetColorBufferFormat();
            data.m_alphaBlendMode           = AlphaBlendMode_Trans;
            data.m_expandConstantBuffer     = &m_cbData;
            data.m_expandConstantBufferSize = sizeof(CbMotionBlur);
            m_blurSprite.Init(data);
        }
    }

    void MotionBlur::Execute(RenderContext& rc)
    {
        const float dt = g_gameTime->GetFrameDeltaTime();
        m_currentStrength += (m_targetStrength - m_currentStrength)
            * min(1.0f, dt * FADE_SPEED);

        if (m_currentStrength < MIN_STRENGTH)
        {
            return;
        }

        // Step 1: メインRT → テンポラリRT へコピー（同一RTを同時読み書きしないため）
        rc.WaitUntilToPossibleSetRenderTarget(m_tempRT);
        rc.SetRenderTargetAndViewport(m_tempRT);
        m_copySprite.Draw(rc);
        rc.WaitUntilFinishDrawingToRenderTarget(m_tempRT);

        // Step 2: テンポラリRTを元にラジアルブラーをメインRTへアルファ合成
        m_cbData.strength = m_currentStrength;
        m_blurSprite.GetExpandConstantBufferGPU().CopyToVRAM(m_cbData);

        rc.WaitUntilToPossibleSetRenderTarget(*m_mainRT);
        rc.SetRenderTargetAndViewport(*m_mainRT);
        m_blurSprite.Draw(rc);
        rc.WaitUntilFinishDrawingToRenderTarget(*m_mainRT);
    }
    
}
