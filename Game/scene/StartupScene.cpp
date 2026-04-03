/**
 * StartupScene.cpp
 * スタートアップシーン
 */
#include "stdafx.h"
#include "StartupScene.h"
#include "TitleScene.h"
#include "sound/SoundManager.h"


namespace
{
	enum EnStartupKind
	{
		enBootKind_Game,
		enBootKind_LogoA,
		enBootKind_LogoB,
		enBootKind_Max,
	};
	struct StartupInformation
	{
		std::string assetPath;
		float time;
		float shortTime;
		int voiceKind;
		//
		StartupInformation(const std::string& path, const float t, const float st, const int vKind)
			: assetPath(path)
			, time(t)
			, shortTime(st)
			, voiceKind(vKind)

		{
		}
	};
	static const StartupInformation sBootInfoList[] = {
		//StartupInformation("Assets/ui/startup/notitle.dds", 2.5f, 0.5f, enSoundKind_None),
		//StartupInformation("Assets/ui/startup/attension.dds", 2.5f, 0.5f, enSoundKind_None),
		StartupInformation("Assets/ui/startup/StartupLogo.dds", 3.0f, 0.5f, static_cast<uint32_t>(app::SoundKind::None)),
		StartupInformation("Assets/ui/startup/kawahara.dds", 3.0f, 0.5f, static_cast<uint32_t>(app::SoundKind::Startup00)),
		StartupInformation("Assets/ui/startup/kbc_games.dds", 3.0f, 0.5f, static_cast<uint32_t>(app::SoundKind::Startup01)),
	};
}


StartupScene::StartupScene()
{
}


StartupScene::~StartupScene()
{
}


bool StartupScene::Start()
{
	// 初期設定
	Change();

	return true;
}


void StartupScene::Update()
{
	if (m_spriteRender) {
		m_spriteRender->Update();
	}
	if (CanChange()) {
		Change();
		m_elapsedTime = 0.0f;
	}
	else {
		const float deltaTime = g_gameTime->GetFrameDeltaTime();
		m_elapsedTime += deltaTime;
	}
}


void StartupScene::Render(RenderContext& rc)
{
	if (m_spriteRender) {
		m_spriteRender->Draw(rc);
	}
}


bool StartupScene::RequestScene(uint32_t& id, float& waitTime)
{
	if (m_currentIndex >= enBootKind_Max) {
		if (CanChange()) {
			id = TitleScene::ID();
			return true;
		}
	}
	return false;
}


void StartupScene::Change()
{
	if (m_spriteRender) {
		delete m_spriteRender;
		m_spriteRender = nullptr;
	}
	if (m_currentIndex >= enBootKind_Max) {
		return;
	}

	const auto& bootInfo = sBootInfoList[m_currentIndex++];

	m_spriteRender = new SpriteRender();
	m_spriteRender->Init(bootInfo.assetPath.c_str(), MAX_SPRITE_WIDTH, MAX_SPRITE_HIGHT);

	m_changeTime = bootInfo.time;
	m_shortChangeTime = bootInfo.shortTime;

	if (bootInfo.voiceKind != static_cast<uint32_t>(app::SoundKind::None)) {
		app::SoundManager::Get().PlaySE(bootInfo.voiceKind);
	}
}


bool StartupScene::CanChange() const
{
	// 指定した経過時間を過ぎているなら変更可能
	if (m_elapsedTime >= m_changeTime) {
		return true;
	}
	// 一定時間を過ぎていて何かボタンを押したなら切り替え可能
	if (m_elapsedTime >= m_shortChangeTime) {
		if (g_pad[0]->IsTrigger(enButtonA)) {
			return true;
		}
	}
	return false;
}