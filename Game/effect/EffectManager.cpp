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
	// 追従エフェクトの座標を毎フレーム同期
	// 終了済みエントリ（K2Engine側で削除済み）は除去する
	m_followEffectList.erase(
		std::remove_if(m_followEffectList.begin(), m_followEffectList.end(),
			[](const FollowEffectEntry& entry)
			{
				return entry.emitter == nullptr || !entry.emitter->IsPlay();
			}),
		m_followEffectList.end()
	);

	for (auto& entry : m_followEffectList)
	{
		if (entry.targetPosition)
		{
			entry.emitter->SetPosition(*entry.targetPosition);
		}
	}
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

EffectHandle EffectManager::PlayEffectFollow(const int kind, const Vector3* targetPosition, const Quaternion& rotation, const Vector3& scale)
{
	if (m_effectHandleCount == INVALID_EFFECT_HANDLE) {
		K2_ASSERT(false, "エフェクトの再生が多いです。\n");
		return INVALID_EFFECT_HANDLE;
	}
	if (targetPosition == nullptr) {
		K2_ASSERT(false, "追従先のポインタがnullptrです。\n");
		return INVALID_EFFECT_HANDLE;
	}

	EffectEmitter* emitter = NewGO<EffectEmitter>(0);
	emitter->Init(kind);
	emitter->SetPosition(*targetPosition);
	emitter->SetRotation(rotation);
	emitter->SetScale(scale);
	emitter->Play();

	FollowEffectEntry entry;
	entry.handle = m_effectHandleCount;
	entry.emitter = emitter;
	entry.targetPosition = targetPosition;
	m_followEffectList.push_back(entry);

	return m_effectHandleCount++;
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
	EffectManager::Get().Update();
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

EffectHandle EffectManagerObject::PlayEffectFollow(const int kind, const Vector3* targetPosition, const Quaternion& rotation, const Vector3& scale)
{
	return EffectManager::Get().PlayEffectFollow(kind, targetPosition, rotation, scale);
}
