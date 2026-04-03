/**
 * Application.cpp
 * アプリケーション制御用クラス群
 * 更新処理やレンダー処理の管理
 */
#include "stdafx.h"
#include "Application.h"
#include "core/Fade.h"
#include "core/ParameterManager.h"
#include "camera/CameraManager.h"
#include "camera/CameraController.h"
#include "scene/SceneManager.h"
#include "sound/SoundManager.h"
#include "collision/GhostBodyManager.h"

// @todo for test
#include "ui/UIAnimationParameter.h"


namespace
{
	//static app::ui::Layout* layout = nullptr;
	//static app::ui::SoundOptionMenu* menu = nullptr;
}


namespace app
{
	Application::Application()
	{
		constexpr size_t HEAP_ALLOCATOR_SIZE = static_cast<size_t>(1024) * 1024 * 1024 * 2; // 2GB
		app::memory::Allocator::Get().Initialize(HEAP_ALLOCATOR_SIZE);

		app::collision::GhostBodyManager::Initialize();
	}


	Application::~Application()
	{
		DeleteGO(m_sceneManager);
		DeleteGO(m_fadeObject);
		app::collision::GhostBodyManager::Finalize();
		app::SoundManager::Finalize();
		app::core::ParameterManager::Finalize();
		

		app::memory::Allocator::Get().Shutdown();
	}


	void Application::Initialize(HWND hwnd)
	{
		// k2EngineLowの初期化。
		k2EngineLow_ = new K2EngineLow();
		k2EngineLow_->Init(hwnd, FRAME_BUFFER_W, FRAME_BUFFER_H);
		g_camera3D->SetPosition(Vector3(0.0f, 100.0f, -200.0f));
		g_camera3D->SetTarget(Vector3(0.0f, 50.0f, 0.0f));

		//シーンライト初期化
		sceneLight_.Init();
		g_sceneLight = &sceneLight_;
		// レンダリングエンジン
		renderingEngine_.Init();
		g_renderingEngine = &renderingEngine_;

		PhysicsWorld::Initialize();

		// ===========================================================
		// ここから下でゲーム固有の初期化処理を行う。
		// ===========================================================
		
		// カメラマネージャー初期化
		app::camera::CameraManager::Initialize();
		app::camera::CameraManager::Get().Setup(g_camera3D);

		// パラメーター管理生成
		app::core::ParameterManager::Initialize();
		// サウンド管理生成
		app::SoundManager::Initialize();
		// Fade処理生成
		m_fadeObject = NewGO<FadeObject>(static_cast<uint8_t>(ObjectPriority::Fade));
		// シーン管理生成
		m_sceneManager = NewGO<SceneManagerObject>(static_cast<uint8_t>(ObjectPriority::Default));

		//UIアニメーションのJSONファイル読み込み
		app::ui::UIAnimationParameter::Get().Load("Assets/master/ui_animation.json");
	}


	void Application::BeginFrame()
	{
		k2EngineLow_->BeginFrame();
	}


	void Application::Update()
	{
		for (auto* pad : g_pad) {
			pad->Update();
		}
		g_soundEngine->Update();
		
		GameObjectManager::GetInstance()->ExecuteUpdate();
		

		// カメラマネージャーの更新。
		app::camera::CameraManager::Get().Update(g_gameTime->GetFrameDeltaTime());

		//物理エンジンのアップデートを呼び出す。
		PhysicsWorld::Get().Update(1.0f / 60.0f);	// 固定値で更新

		// エフェクトエンジンの更新。
		EffectEngine::GetInstance()->Update(g_gameTime->GetFrameDeltaTime());

		SoundManager::Get().Update();

#ifdef APP_ENABLE_PARAM_HOT_RELOAD
		app::core::ParameterManager::Get().Update();
#endif


#if defined(APP_DEBUG)
		static bool isTriggerDebugCameraKey = false;
		if (GetAsyncKeyState(VK_F2)) {
			if (!isTriggerDebugCameraKey) {
				static bool isUseDebugCamera = false;
				if (!isUseDebugCamera) {
					app::camera::RefCameraController debugCamera = std::make_shared<app::camera::DebugCamera>();
					app::camera::CameraManager::Get().SwitchCamera(debugCamera);
				}
				else {
					app::camera::CameraManager::Get().SwitchPrevCamera();
				}
				isUseDebugCamera = !isUseDebugCamera;
				isTriggerDebugCameraKey = true;
			}
		}
		else {
			isTriggerDebugCameraKey = false;
		}
#endif// APP_DEBUG
	}


	void Application::Render(RenderContext& rc)
	{
		// ゲームオブジェクトマネージャーの描画処理を実行。
		GameObjectManager::GetInstance()->ExecuteRender(rc);

		//ライトの更新処理(この間でする)
		g_sceneLight->Update();
		
		//レンダリングエンジンの描画処理
		g_renderingEngine->Execute(rc);
	}


	void Application::RenderDebug(RenderContext& rc)
	{
#if defined(K2_DEBUG)
		//当たり判定描画処理を実行。
		PhysicsWorld::Get().DebubDrawWorld(rc);
#endif
	}


	void Application::EndFrame()
	{
		k2EngineLow_->EndFrame();
	}


	void Application::Finalize()
	{
		PhysicsWorld::Finalize();

		delete k2EngineLow_;
	}
}