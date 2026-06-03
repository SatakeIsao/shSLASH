#include "stdafx.h"
#include "SceneManager.h"

#include "StartupScene.h"
#include "TitleScene.h"
#include "BootScene.h"
#include "BattleScene.h"
#include "GameOverScene.h"
#include "ResultScene.h"

#include "core/Fade.h"


SceneManager* SceneManager::m_instance = nullptr;	// 初期化


SceneManager::SceneManager()
{
	AddSceneMap<StartupScene>();
	AddSceneMap<TitleScene>();

#if defined(APP_DEBUG)
	AddSceneMap<BootScene>();
#endif

	AddSceneMap<BattleScene>();
	AddSceneMap<GameOverScene>();
	AddSceneMap<ResultScene>();
}


SceneManager::~SceneManager()
{
}


void SceneManager::Update()
{
	if (m_currentScene) {
		m_currentScene->Update();

		if (!m_isFadingOut) {
			if (m_currentScene->RequestScene(m_pendingSceneId, m_pendingWaitTime)) {
				m_isFadingOut = true;
				Fade::Get().FadeOut(0.5f);
			}
		}

		if (m_isFadingOut && Fade::Get().IsFadeOutComplete()) {
			delete m_currentScene;
			m_currentScene = nullptr;
			m_isFadingOut = false;
			nextSceneId_ = m_pendingSceneId;
			m_waitTime = m_pendingWaitTime;
			m_pendingSceneId = INVALID_SCENE_ID;
			m_pendingWaitTime = 0.0f;
			Fade::Get().Enable();
		}
	}

	if (nextSceneId_ != INVALID_SCENE_ID) {
		m_elapsedTime += g_gameTime->GetFrameDeltaTime();
		if (m_elapsedTime >= m_waitTime) {
			CreateScene(nextSceneId_);
			m_waitTime = 0.0f;
			m_elapsedTime = 0.0f;
			nextSceneId_ = INVALID_SCENE_ID;
			m_isFadingIn = true;
			Fade::Get().FadeIn(0.5f);
		}
	}

	if (m_isFadingIn && Fade::Get().IsFadeInComplete()) {
		m_isFadingIn = false;
		Fade::Get().Disable();
	}
}


void SceneManager::Render(RenderContext& rc)
{
	if (m_currentScene) {
		m_currentScene->Render(rc);
	}
}


void SceneManager::CreateScene(const uint32_t id)
{
	auto it = m_sceneMap.find(id);
	if (it == m_sceneMap.end()) {
		K2_ASSERT(false, "新規シーンが追加されていません。\n");
	}
	auto& createSceneFunc = it->second;
	m_currentScene = createSceneFunc();
	m_currentScene->Start();
}




/*****************************************************/


SceneManagerObject::SceneManagerObject()
{
	SceneManager::Initialize();
}


SceneManagerObject::~SceneManagerObject()
{
	SceneManager::Finalize();
}


bool SceneManagerObject::Start()
{
	// 最初のシーンを設定
//#if defined(APP_DEBUG)
//	SceneManager::Get().CreateScene(DebugScene::ID());
//#else
	//SceneManager::Get().CreateScene(StartupScene::ID());
//#endif // APP_DEBUG

	/** デバックテスト */
	SceneManager::Get().CreateScene(TitleScene::ID());

	return true;
}


void SceneManagerObject::Update()
{
	SceneManager::Get().Update();
}


void SceneManagerObject::Render(RenderContext& rc)
{
	SceneManager::Get().Render(rc);
}