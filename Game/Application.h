/**
 * Application.h
 * アプリケーション制御用クラス群
 * 更新処理やレンダー処理の管理
 */
#pragma once


 /** 基底クラス */
class IApplication : public Noncopyable
{
public:
	IApplication() {}
	virtual ~IApplication() {}


	virtual void Initialize(HWND hwnd) = 0;
	
	virtual void BeginFrame() = 0;

	virtual void Update() = 0;
	
	virtual void Render(RenderContext& rc) = 0;
	
	virtual void RenderDebug(RenderContext& rc) = 0;

	virtual void EndFrame() = 0;

	virtual void Finalize() = 0;
};


class SceneManagerObject;
class FadeObject;

namespace app
{
	class Application : public IApplication
	{
	protected:
		K2EngineLow* k2EngineLow_ = nullptr;
		SceneLight sceneLight_;
		RenderingEngine renderingEngine_;

		SceneManagerObject* m_sceneManager = nullptr;
		FadeObject* m_fadeObject = nullptr;


	public:
		Application();
		virtual ~Application();


		virtual void Initialize(HWND hwnd) override;

		virtual void BeginFrame() override;

		virtual void Update() override;
		
		virtual void Render(RenderContext& rc) override;
		
		virtual void RenderDebug(RenderContext& rc) override;

		virtual void EndFrame() override;

		virtual void Finalize() override;
	};
}