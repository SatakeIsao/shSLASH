/**
 * EffectManager.cpp
 * エフェクト管理
 * 必要なエフェクトファイルを読み込んだり再生したりなど管理する
 */
#include "stdafx.h"
#include "effect/EffectManager.h"


EffectManager* EffectManager::m_instance = nullptr; //初期化


EffectManager::EffectManager()
{
	m_effectList.clear();

	// サウンドの登録
	for (int i = 0; i < ARRAYSIZE(effectInformation); ++i) {
		const auto& info = effectInformation[i];
		EffectEngine::GetInstance()->ResistEffect(i, info.assetPath);
	}
}


EffectManager::~EffectManager()
{
}


void EffectManager::Update()
{
}


EffectHandle EffectManager::PlayEffect(const int kind, const Vector3& position, const Quaternion& rotation, const Vector3& scale)
{
	// ハンドルが最大数になったら使えない
	// NOTE: そんなに再生するはずがない
	if (m_effectHandleCount == INVALID_EFFECT_HANDLE) {
		K2_ASSERT(false, "エフェクトの再生が多いです。\n");
		return INVALID_EFFECT_HANDLE;
	}
	EffectEmitter* m_effect = NewGO<EffectEmitter>(0);
	m_effect->Init(kind);
	m_effect->SetPosition(position);
	m_effect->SetRotation(rotation);
	m_effect->SetScale(scale);
	m_effect->Play();

	return m_effectHandleCount;
}


void EffectManager::StopEffect(const EffectHandle handle)
{
	auto* effect = FindEffect(handle);
	if (effect == nullptr) {
		return;
	}
	effect->Stop();
}




/***********************************************/


EffectManagerObject::EffectManagerObject()
{
}


EffectManagerObject::~EffectManagerObject()
{
	EffectManager::Finalize();
}


bool EffectManagerObject::Start()
{
	EffectManager::Initialize();

	return true;
}


void EffectManagerObject::Update()
{
}


void EffectManagerObject::Render(RenderContext& rc)
{
}


EffectHandle EffectManagerObject::PlayEffect(const int kind, const Vector3& position, const Quaternion& rotation, const Vector3& scale)
{
	/** 処理はEffectManagerに任せる */
	return EffectManager::Get().PlayEffect(kind, position, rotation, scale);
}


void EffectManagerObject::StopEffect(const EffectHandle handle)
{
	/** 処理はEffectManagerに任せる */
	EffectManager::Get().StopEffect(handle);
}
