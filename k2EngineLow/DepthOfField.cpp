#include "k2EngineLowPreCompile.h"
#include "DepthOfField.h"

namespace
{
    /** 近景ボケ距離 */
    constexpr float NEAR_BLUR = 50.0f;
    /** 遠景ボケ距離 */
    constexpr float FAR_BLUR = 2000.0f;
    /** ボケ効果が切り替わる速度 */
    constexpr float FADE_SPEED = 3.0f;
    /** プレイヤーの判定半径 */
    constexpr float PLAYER_RADIUS = 80.0f;
    /** ブラーの強度 */
    constexpr float BLUR_POWER = 20.0f;
    /** ボケ効果の最小閾値 */
    constexpr float MIN_STRENGTH = 0.001f;
}

namespace nsK2EngineLow {

    void DepthOfField::Init(RenderTarget& mainRT, Texture& worldPosTexture)
    {
        m_mainRT = &mainRT;

        m_blur.Init(&mainRT.GetRenderTargetTexture());

        memset(&m_cbData, 0, sizeof(m_cbData));
        m_cbData.nearBlur     = NEAR_BLUR;
        m_cbData.farBlur      = FAR_BLUR;
        m_cbData.playerRadius = PLAYER_RADIUS;

        SpriteInitData spriteInitData;
        spriteInitData.m_textures[0]              = &m_blur.GetBokeTexture();
        spriteInitData.m_textures[1]              = &worldPosTexture;
        spriteInitData.m_width                    = mainRT.GetWidth();
        spriteInitData.m_height                   = mainRT.GetHeight();
        spriteInitData.m_fxFilePath               = "Assets/Shader/dof.fx";
        spriteInitData.m_colorBufferFormat[0]     = mainRT.GetColorBufferFormat();
        spriteInitData.m_alphaBlendMode           = AlphaBlendMode_Trans;
        spriteInitData.m_expandConstantBuffer     = &m_cbData;
        spriteInitData.m_expandConstantBufferSize = sizeof(CbDoF);

        m_dofSprite.Init(spriteInitData);
    }

    void DepthOfField::Execute(RenderContext& rc)
    {
        const float dt = g_gameTime->GetFrameDeltaTime();
        m_currentStrength += (m_targetStrength - m_currentStrength)
                             * min(1.0f, dt * FADE_SPEED);

        if (m_currentStrength < MIN_STRENGTH)
        {
            return;
        }

        m_blur.ExecuteOnGPU(rc, BLUR_POWER);

        const Vector3 pos = g_camera3D->GetPosition();
        const Vector3 fwd = g_camera3D->GetForward();
        m_cbData.eyePos         = Vector4(pos.x, pos.y, pos.z, 1.0f);
        m_cbData.viewForward    = Vector4(fwd.x, fwd.y, fwd.z, 0.0f);
        m_cbData.playerWorldPos = Vector4(m_playerWorldPos.x, m_playerWorldPos.y, m_playerWorldPos.z, 1.0f);
        m_cbData.nearBlur       = NEAR_BLUR;
        m_cbData.farBlur        = FAR_BLUR;
        m_cbData.strength       = m_currentStrength;
        m_cbData.playerRadius   = PLAYER_RADIUS;

        m_dofSprite.GetExpandConstantBufferGPU().CopyToVRAM(m_cbData);

        rc.WaitUntilToPossibleSetRenderTarget(*m_mainRT);
        rc.SetRenderTargetAndViewport(*m_mainRT);
        m_dofSprite.Draw(rc);
        rc.WaitUntilFinishDrawingToRenderTarget(*m_mainRT);
    }
}
