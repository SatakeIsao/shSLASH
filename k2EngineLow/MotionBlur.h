#pragma once

namespace nsK2EngineLow {

    class MotionBlur : public Noncopyable
    {
    public:
        struct CbMotionBlur
        {
            float   strength;
            float   blurAmount;
            Vector2 center;
        };

        void Init(RenderTarget& mainRT);
        void Execute(RenderContext& rc);

        void SetEnabled(bool enabled)
        {
            m_targetStrength = enabled ? 1.0f : 0.0f;
        }

        bool IsEnabled() const { return m_targetStrength > 0.5f; }

    private:
        RenderTarget  m_tempRT;
        Sprite        m_copySprite;
        Sprite        m_blurSprite;
        CbMotionBlur  m_cbData;

        float         m_currentStrength = 0.0f;
        float         m_targetStrength  = 0.0f;

        RenderTarget* m_mainRT = nullptr;
    };
}
