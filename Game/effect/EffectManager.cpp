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
	// ResistEffect はEffectManagerObject::Start()で分割登録済み。
}


EffectManager::~EffectManager()
{
	for (auto* emitter : m_activeEffectList)
	{
		if (emitter && emitter->IsPlay())
			emitter->Stop();
	}
	m_activeEffectList.clear();

	for (auto& entry : m_followEffectList)
	{
		if (entry.emitter && entry.emitter->IsPlay())
			entry.emitter->Stop();
	}
	m_followEffectList.clear();
}


void EffectManager::Update()
{
	// 終了済みの非追従エフェクトをリストから除去
	m_activeEffectList.erase(
		std::remove_if(m_activeEffectList.begin(), m_activeEffectList.end(),
			[](EffectEmitter* e) { return e == nullptr || !e->IsPlay(); }),
		m_activeEffectList.end()
	);

	// 再生終了したエフェクトを m_effectList からも除去
	for (auto it = m_effectList.begin(); it != m_effectList.end(); )
	{
		if (it->second == nullptr || !it->second->IsPlay())
			it = m_effectList.erase(it);
		else
			++it;
	}

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


EffectHandle EffectManager::PlayEffect(const int kind, const Vector3& position, const Quaternion& rotation, const Vector3& scale, float speed)
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
	if (speed != 1.0f)
	{
		m_effect->SetSpeed(speed);
	}
	m_effectList[m_effectHandleCount] = m_effect;
	m_activeEffectList.push_back(m_effect);

	return m_effectHandleCount++;
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
	auto it = m_effectList.find(handle);
	if (it == m_effectList.end()) return;
	if (it->second) it->second->Stop();
	m_effectList.erase(it);
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
	const int total = static_cast<int>(enEffectKind_Max);
	for (int i = 0; i < total; ++i)
		EffectEngine::GetInstance()->ResistEffect(i, effectInformation[i].assetPath);
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
