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

	m_iconRender.Init("Assets/ui/LoadingIcon.dds", 128.0f, 128.0f);
	m_iconRender.SetPosition(Vector3(700.0f, -350.0f, 0.0f));

	m_iconConputeRate.Initialize(ICON_ROTATE_TIME, true);
}


Fade::~Fade()
{
}


void Fade::FadeOut(float duration)
{
	m_fadeOutDuration = max(duration, 0.001f);
	m_fadeOutElapsed  = 0.0f;
	isFadingOut       = true;
	isFadingIn        = false;
	isEnable          = true;
	m_fadeAlpha       = 0.0f;
}


void Fade::FadeIn(float duration)
{
	m_fadeInDuration = max(duration, 0.001f);
	m_fadeInElapsed  = 0.0f;
	isFadingIn       = true;
	isFadingOut      = false;
	m_fadeAlpha      = 1.0f;
}


void Fade::Update()
{
	if (!isEnable) {
		return;
	}

	if (isFadingOut) {
		m_fadeOutElapsed += g_gameTime->GetFrameDeltaTime();
		m_fadeAlpha = min(1.0f, m_fadeOutElapsed / m_fadeOutDuration);
		if (m_fadeAlpha >= 1.0f) {
			isFadingOut = false;
		}
	}
	else if (isFadingIn) {
		m_fadeInElapsed += g_gameTime->GetFrameDeltaTime();
		m_fadeAlpha = max(0.0f, 1.0f - m_fadeInElapsed / m_fadeInDuration);
		if (m_fadeAlpha <= 0.0f) {
			isFadingIn = false;
		}
	}

	m_fadeRender.SetMulColor(Vector4(0.0f, 0.0f, 0.0f, m_fadeAlpha));
	m_fadeRender.Update();

	if (!isFadingOut && !isFadingIn) {
		const float rate = m_iconConputeRate.Update();
		const float rotate = Math::Lerp(rate, 0.0f, Math::PI2);
		Quaternion q;
		q.SetRotationZ(rotate);
		m_iconRender.SetRotation(q);
		m_iconRender.Update();
	}
}


void Fade::Render(RenderContext& rc)
{
	if (!isEnable) {
		return;
	}
	m_fadeRender.Draw(rc);
	if (!isFadingOut && !isFadingIn) {
		m_iconRender.Draw(rc);
	}
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
