#include "stdafx.h"
#include "ParameterManager.h"


namespace app
{
	namespace core
	{
		ParameterManager* ParameterManager::m_instance = nullptr;	//初期化

		ParameterManager::ParameterManager()
		{

		}

		ParameterManager::~ParameterManager()
		{
			//動的確保したパラメーターを全解放
			for (auto& it : m_parameterMap)
			{
				auto& parameters = it.second;
				for (auto* parameter : parameters)
				{
					delete parameter;
				}
				parameters.clear();
			}
			m_parameterMap.clear();
		}

	}
}