#pragma once

namespace nsK2EngineLow {
	class PointLight: public Noncopyable
	{
	public:
		PointLight();
		~PointLight();

		////座標を設定
		//void SetPosition(const Vector3& position)
		//{                                                                                           
		//m_pointLight->SetPosition(position);
		//}
		//void SetPosition(float x, float y, float z)
		//{
		//	m_pointLight->SetPosition(x, y, z);
		//}

		////カラーを設定
		//void SetColor(const Vector3& color)
		//{
		//	m_pointLight->SetColor(color);
		//}
		//void SetColor(float r, float g, float b)
		//{
		//	m_pointLight->SetColor(r, g, b);
		//}

		////範囲を設定
		//void SetRange(float range)
		//{
		//	m_pointLight->SetRange(range);
		//}

		////影響率の累乗数を設定
		//void SetAffectPowParam(float powParam)
		//{
		//	m_pointLight->SetAffectPowParam(powParam);
		//}

		////座標を取得
		//const Vector3& GetPosition() const
		//{
		//	return m_pointLight->GetPosition();
		//}
		////カラーを取得
		//const Vector3& GetColor() const
		//{
		//	return m_pointLight->GetColor();
		//}
		////影響範囲を取得
		//float GetRange() const
		//{
		//	return m_pointLight->GetRange();
		//}
		////生データを取得
		//const SPointLight& GetRawData() const
		//{
		//	return *m_pointLight;
		//}

	private:
		//SPointLight* m_pointLight = nullptr;
	};
}


