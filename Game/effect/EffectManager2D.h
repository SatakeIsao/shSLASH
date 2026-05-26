/**
 * EffectManager2D.h
 * EffectEmitter ベースの再生時間管理ラッパー
 */
#pragma once

/** 2Dエフェクトの種類 */
enum enEffectKind2D
{
    enEffectKind2D_PlayerUIHealHP = 0,
    enEffectKind2D_Max,
};


class EffectManager2D
{
private:
    EffectManager2D() = default;
    ~EffectManager2D() = default;

public:
    void PlayEffect(int kind, const Vector3& pos, const Vector3& scale = Vector3::One);

    static void Initialize();
    static EffectManager2D& Get();
    static bool IsAvailable();
    static void Finalize();

private:
    static EffectManager2D* m_instance;
};


class EffectManager2DObject : public IGameObject
{
public:
    ~EffectManager2DObject();
    bool Start() override;
    void Update() override {}

    void PlayEffect(int kind, const Vector3& pos, const Vector3& scale = Vector3::One);
};
