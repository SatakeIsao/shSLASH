/**
 * SoundManager.cpp
 * サウンド管理
 * 必要なサウンドファイルを読み込んだり再生したりなど管理する
 */
#include "stdafx.h"
#include "SoundManager.h"

namespace app
{

	SoundManager* SoundManager::m_instance = nullptr; //初期化


	SoundManager::SoundManager()
	{
		m_seList.clear();

		// サウンドの登録
		for (int i = 0; i < ARRAYSIZE(soundInformation); ++i) {
			const auto& info = soundInformation[i];
			g_soundEngine->ResistWaveFileBank(i, info.assetPath.c_str());
		}
	}


	SoundManager::~SoundManager()
	{
	}


	void SoundManager::Update()
	{
		// SEリストから再生していないものがあれば削除する
		std::vector<SoundHandle> eraseList;
		for (auto& it : m_seList) {
			const auto key = it.first;
			auto* se = it.second;
			// 再生が終わっているなら削除
			if (!se->IsPlaying()) {
				eraseList.push_back(key);
			}
		}
		for (const auto& key : eraseList) {

			m_seList.erase(key);
		}
	}


	void SoundManager::SetVolume(const SoundVolumeType volumeType, const float volume)
	{
		switch (volumeType)
		{
			case SoundVolumeType::Master:
			{
				volumes[static_cast<uint8_t>(volumeType)] = volume;
				SetVolume(SoundVolumeType::BGM, volumes[static_cast<uint8_t>(SoundVolumeType::BGM)]);
				SetVolume(SoundVolumeType::SE, volumes[static_cast<uint8_t>(SoundVolumeType::SE)]);
				break;
			}
			case SoundVolumeType::BGM:
			{
				volumes[static_cast<uint8_t>(SoundVolumeType::BGM)] = volume;
				// マスター(90%) * BGM
				// 0.9 * 0.8 = 0.72
				SetVolumeBGM(ComputeOutputVolume(SoundVolumeType::BGM));
				break;
			}
			case SoundVolumeType::SE:
			{
				volumes[static_cast<uint8_t>(SoundVolumeType::SE)] = volume;
				// マスター(90%) * SE
				// 0.9 * 0.8 = 0.72
				for (auto& it : m_seList) {
					it.second->SetVolume(ComputeOutputVolume(SoundVolumeType::SE));
				}
				break;
			}
			default:
			{
				K2_ASSERT(false, "処理が追加されていません\n");
				break;
			}
		}
	}


	void SoundManager::PlayBGM(const int kind)
	{
		// BGMが生成されていない
		if (m_bgm == nullptr) {
			// 生成
			m_bgm = NewGO<SoundSource>(0, "bgm");
		}
		else {
			// すでに生成されているならBGMを停止する
			m_bgm->Stop();
		}
		// 初期化
		m_bgm->Init(kind);
		m_bgm->Play(true);	// BGMなのでループ再生する

	}


	void SoundManager::StopBGM()
	{
		if (m_bgm == nullptr) {
			return;
		}
		m_bgm->Stop();
	}


	void SoundManager::SetVolumeBGM(const float volume)
	{
		if (m_bgm != nullptr)
		{
			m_bgm->SetVolume(volume);
		}
	}


	SoundHandle SoundManager::PlaySE(const int kind, const bool isLood, const bool is3D)
	{
		// ハンドルが最大数になったら使えない
		// NOTE: そんなに再生するはずがない
		if (m_soundHandleCount == INVALID_SOUND_HANDLE) {
			K2_ASSERT(false, "サウンドの再生が多いです。\n");
			return INVALID_SOUND_HANDLE;
		}
		auto* se = NewGO<SoundSource>(0, "se");
		se->Init(kind, is3D);
		se->Play(isLood);
		se->SetVolume(ComputeOutputVolume(SoundVolumeType::SE));
		m_seList.emplace(m_soundHandleCount++, se);

		return m_soundHandleCount;
	}


	void SoundManager::StopSE(const SoundHandle handle)
	{
		auto* se = FindSE(handle);
		if (se == nullptr) {
			return;
		}
		se->Stop();
	}


	void SoundManager::SetVolumeSE(const SoundHandle handle, float volume)
	{
		auto* se = FindSE(handle);
		if (se == nullptr) {
			return;
		}
		se->SetVolume(volume);
	}
}