/**
 * Fade.h
 */
#pragma once


class Fade
{
private:
	SpriteRender m_fadeRender;
	SpriteRender m_iconRender;
	bool isEnable    = false;
	bool isFadingOut = false;
	bool isFadingIn  = false;
	float m_fadeAlpha       = 0.0f;
	float m_fadeOutDuration = 0.0f;
	float m_fadeOutElapsed  = 0.0f;
	float m_fadeInDuration  = 0.0f;
	float m_fadeInElapsed   = 0.0f;

	app::ComputeRate m_iconConputeRate;


private:
	Fade();
	~Fade();


public:
	void Update();
	void Render(RenderContext& rc);


public:
	void Enable()
	{
		isEnable    = true;
		isFadingOut = false;
		isFadingIn  = false;
		m_fadeAlpha = 1.0f;
	}
	void Disable()
	{
		isEnable    = false;
		isFadingOut = false;
		isFadingIn  = false;
		m_fadeAlpha = 0.0f;
	}

	void FadeOut(float duration);
	bool IsFadeOutComplete() const { return isEnable && !isFadingOut && m_fadeAlpha >= 1.0f; }

	void FadeIn(float duration);
	bool IsFadeInComplete() const { return isEnable && !isFadingIn && m_fadeAlpha <= 0.0f; }



	/**
	 * singleton
	 */
private:
	static Fade* m_instance;


public:
	static void Create()
	{
		if (m_instance == nullptr)
		{
			m_instance = new Fade();
		}
	}
	static Fade& Get()
	{
		return *m_instance;
	}
	static void Delete()
	{
		if (m_instance != nullptr)
		{
			delete m_instance;
			m_instance = nullptr;
		}
	}
};




/***************************/


class FadeObject : public IGameObject
{
public:
	FadeObject();
	virtual ~FadeObject();

	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;
};
