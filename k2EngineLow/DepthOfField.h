#pragma once
#include "graphics/GaussianBlur.h"

namespace nsK2EngineLow {

    class DepthOfField : public Noncopyable
    {
    public:
        struct CbDoF
        {
            Vector4 eyePos;
            Vector4 viewForward;
            Vector4 playerWorldPos;
            float   nearBlur;
            float   farBlur;
            float   strength;
            float   playerRadius;
        };

        void Init(RenderTarget& mainRT, Texture& worldPosTexture);
        void Execute(RenderContext& rc);

        void SetEnabled(bool enabled)
        {
            m_targetStrength = enabled ? 1.0f : 0.0f;
        }

        bool IsEnabled() const { return m_targetStrength > 0.5f; }

        void SetPlayerWorldPos(const Vector3& pos)
        {
            m_playerWorldPos = pos;
        }

        void SetForceFullBlur(bool v) { m_forceFullBlur = v; }

    private:
        GaussianBlur  m_blur;
        Sprite        m_dofSprite;
        CbDoF         m_cbData;

        float   m_currentStrength = 0.0f;
        float   m_targetStrength  = 0.0f;
        Vector3 m_playerWorldPos;
        bool    m_forceFullBlur  = false;

        RenderTarget* m_mainRT = nullptr;
    };

} // namespace nsK2EngineLow
