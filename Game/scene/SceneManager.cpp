#include "stdafx.h"
#include "SceneManager.h"

#include "StartupScene.h"
#include "TitleScene.h"
#include "BootScene.h"
#include "BattleScene.h"
#include "GameOverScene.h"
#include "GameClearScene.h"

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
	AddSceneMap<GameClearScene>();
}


SceneManager::~SceneManager()
{
}


void SceneManager::Update()
{
	if (m_currentScene) {
		m_currentScene->Update();
		if (m_currentScene->RequestScene(nextSceneId_, m_waitTime)) {
			delete m_currentScene;
			m_currentScene = nullptr;

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

			Fade::Get().Disable();
		}
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
	SceneManager::Get().CreateScene(BattleScene::ID());

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