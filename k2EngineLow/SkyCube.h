#pragma once

namespace nsK2EngineLow {
	enum EnSkyCubeType
	{
		enSkyCubeType_Day,				// 昼間
		enSkyCubeType_Night,			// 夜間
		enSkyCubeType_Snow,				// 雪山
		enSkyCubeType_Snow_2,			// 雪山_2
		enSkyCubeType_Wild,				// 荒野
		enSkyCubeType_Wild_2,			// 荒野２
		enSkyCubeType_Wild_Night,		// 荒野(夜間)
		enSkyCubeType_Grass,			// 芝生
		enSkyCubeType_Euro,				// 欧州
		enSkyCubeType_DayToon,			// 昼間(トゥーン調)
		enSkyCubeType_DayToon_2,		// 昼間(トゥーン調)２
		enSkyCubeType_DayToon_3,		// 昼間(トゥーン調)３
		enSkyCubeType_DayToon_4,		// 昼間(トゥーン調)４
		enSkyCubeType_NightToon,		// 夜間(トゥーン調)
		enSkyCubeType_NightToon_2,		// 夜間(トゥーン調)２
		enSkyCubeType_SunriseToon,		// 明け方(トゥーン調)
		//enSkyCubeType_SpaceToon,		// 大気圏(トゥーン調)
		enSkyCubeType_SpaceToon_2,		// 大気圏(トゥーン調)２
		enSkyCubeType_Num,

	};
	class SkyCube : public IGameObject
	{
	public:
		SkyCube();
		~SkyCube();
		bool Start() override final;
		void Update() override final;
		void Render(RenderContext& rc)override final;

	public:
		void SetPosition(const Vector3& pos)
		{
			m_position = pos;
			m_isDirty = true;
		}

		void SetScale(const Vector3& scale)
		{
			m_scale = scale;
			m_isDirty = true;
		}
		void SetScale(const float scale)
		{
			m_scale = g_vec3One;
			m_scale.Scale(scale);
			m_isDirty = true;
		}
		/// <summary>
		/// スカイキューブのタイプを設定
		/// </summary>
		/// <param name="type"></param>
		void SetType(EnSkyCubeType type)
		{
			m_type = type;
		}
		void SetLuminance(float lum)
		{
			m_luminance = lum;
		}
	private:
		ModelRender m_modelRender;
		Texture m_texture[enSkyCubeType_Num];
		Vector3 m_position = g_vec3Zero;
		Vector3 m_scale = g_vec3One * 1000.0f;
		EnSkyCubeType m_type = enSkyCubeType_Day;
		Quaternion m_rot = { 0.0f,0.0f,0.0f,1.0f };
		const wchar_t* m_textureFilePaths[enSkyCubeType_Num];
		float m_luminance = 1.0f;
		bool m_isDirty;
	};
}


