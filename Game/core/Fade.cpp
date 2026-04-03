#include "stdafx.h"
#include "Fade.h"


namespace
{
	constexpr float ICON_ROTATE_TIME = 2.0f;
}


Fade* Fade::m_instance = nullptr;


Fade::Fade()
{
	m_fadeRender.Init("Assets/ui/fade.dds", 1920.0f, 1080.0f);
	m_fadeRender.SetMulColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	m_iconRender.Init("Assets/ui/LoadingIcon.dds", 128.0f, 128.0f);
	m_iconRender.SetPosition(Vector3(700.0f, -350.0f, 0.0f));

	m_iconConputeRate.Initialize(ICON_ROTATE_TIME, true);
}


Fade::~Fade()
{
}


void Fade::Update()
{
	if (!isEnable) {
		return;
	}
	// フェード背景更新
	m_fadeRender.Update();
	// アイコン更新
	{
		const float rate = m_iconConputeRate.Update();
		const float rotate = Math::Lerp(rate, 0.0f, Math::PI2);
		Quaternion q;
		q.SetRotationZ(rotate);
		m_iconRender.SetRotation(q);
	}
	m_iconRender.Update();
}


void Fade::Render(RenderContext& rc)
{
	if (!isEnable) {
		return;
	}
	m_fadeRender.Draw(rc);
	m_iconRender.Draw(rc);
}




/***************************/


FadeObject::FadeObject()
{
	Fade::Create();
}


FadeObject::~FadeObject()
{
	Fade::Delete();
}


bool FadeObject::Start()
{	
	return true;
}


void FadeObject::Update()
{
	Fade::Get().Update();
}


void FadeObject::Render(RenderContext& rc)
{
	Fade::Get().Render(rc);
}