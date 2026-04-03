#include "stdafx.h"
#include "system/system.h"
#include "Application.h"


/// <summary>
/// メイン関数
/// </summary>
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	// ゲームの初期化。
	InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Bario"));
	
	IApplication* application = new app::Application();
	application->Initialize(g_hWnd);
	
	// ここからゲームループ。
	while (DispatchWindowMessage())
	{
		// フレームの開始時に呼び出す必要がある処理を実行
		application->BeginFrame();
		// 更新
		application->Update();
		// 描画
		application->Render(g_graphicsEngine->GetRenderContext());
		// デバッグ描画
		application->RenderDebug(g_graphicsEngine->GetRenderContext());
		// フレームの終了時に呼び出す必要がある処理を実行。
		application->EndFrame();
	}

	delete application;

	return 0;
}

