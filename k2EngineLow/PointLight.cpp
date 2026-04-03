#include "k2EngineLowPreCompile.h"
#include "PointLight.h"

namespace nsK2EngineLow {

	PointLight::PointLight()
	{
		/*if (m_pointLight == nullptr)
		{
			m_pointLight = g_sceneLight->NewPointLight();
		}*/
	}

	PointLight::~PointLight()
	{
		//g_sceneLight->DeletePointLight(m_pointLight);
	}
}