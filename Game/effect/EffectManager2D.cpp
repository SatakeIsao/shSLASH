/**
 * EffectManager2D.cpp
 */
#include "stdafx.h"
#include "effect/EffectManager2D.h"

EffectManager2D* EffectManager2D::m_instance = nullptr;

namespace
{
    struct EffectInfo2D
    {
        const char* texturePath;
        float  texWidth;
        float  texHeight;
        float  duration;
        float  startScale;
        float  endScale;
        int    count;
        float  ringDelay;
    };

    const EffectInfo2D k_effectInfo[enEffectKind2D_Max] =
    {
        // enEffectKind2D_PlayerUIHealHP
        { "Assets/ui/levelUp/Particle01.DDS", 128.0f, 128.0f, 0.7f, 0.5f, 2.8f, 3, 0.1f },
    };
}


class EffectInstance2D : public IGameObject
{
private:
    SpriteRender m_sprite;
    float        m_startS   = 0.5f;
    float        m_endS     = 4.0f;
    float        m_timer    = 0.0f;
    float        m_duration = 0.7f;
    float        m_delay    = 0.0f;

public:
    void Init(const EffectInfo2D& info, const Vector3& pos, float delay)
    {
        m_startS   = info.startScale;
        m_endS     = info.endScale;
        m_duration = info.duration;
        m_delay    = delay;

        m_sprite.Init(info.texturePath, info.texWidth, info.texHeight);
        m_sprite.SetPosition(pos);
        m_sprite.SetScale(m_startS);
        m_sprite.SetMulColor({ 1.0f, 1.0f, 1.0f, 0.0f });
    }

    void Update() override
    {
        m_timer += g_gameTime->GetFrameDeltaTime();

        float elapsed = m_timer - m_delay;
        if (elapsed >= 0.0f && elapsed < m_duration)
        {
            float t     = elapsed / m_duration;
            float eased = 1.0f - (1.0f - t) * (1.0f - t);

            float sprScale = m_startS + (m_endS - m_startS) * eased;
            float sprAlpha = (t < 0.05f) ? (t / 0.05f) : (1.0f - (t - 0.05f) / 0.95f);

            m_sprite.SetScale(sprScale);
            m_sprite.SetMulColor({ 1.0f, 1.0f, 1.0f, sprAlpha });
        }

        m_sprite.Update();

        if (elapsed >= m_duration)
        {
            DeleteGO(this);
        }
    }

    void Render(RenderContext& rc) override
    {
        m_sprite.Draw(rc);
    }
};


void EffectManager2D::Initialize()
{
    if (m_instance == nullptr) m_instance = new EffectManager2D();
}

EffectManager2D& EffectManager2D::Get()
{
    return *m_instance;
}

bool EffectManager2D::IsAvailable()
{
    return m_instance != nullptr;
}

void EffectManager2D::Finalize()
{
    delete m_instance;
    m_instance = nullptr;
}

void EffectManager2D::PlayEffect(int kind, const Vector3& pos, const Vector3& /*scale*/)
{
    if (kind < 0 || kind >= enEffectKind2D_Max) return;

    const auto& info = k_effectInfo[kind];
    for (int i = 0; i < info.count; i++)
    {
        float delay = info.ringDelay * static_cast<float>(i);
        auto* inst = NewGO<EffectInstance2D>(95);
        inst->Init(info, pos, delay);
    }
}


EffectManager2DObject::~EffectManager2DObject()
{
    EffectManager2D::Finalize();
}

bool EffectManager2DObject::Start()
{
    EffectManager2D::Initialize();
    return true;
}

void EffectManager2DObject::PlayEffect(int kind, const Vector3& pos, const Vector3& scale)
{
    EffectManager2D::Get().PlayEffect(kind, pos, scale);
}
