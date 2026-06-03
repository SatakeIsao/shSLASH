/*!
 * @brief	エフェクト
 */

#include "k2EngineLowPreCompile.h"
#include "Effect.h"

namespace nsK2EngineLow {

	Effect::Effect()
	{
	}
	Effect::~Effect()
	{

	}


	void Effect::Init(const int number)
	{
		m_effect = EffectEngine::GetInstance()->LoadEffect(number);

	}
	void Effect::Play()
	{
		//再生中のエフェクトを止める
		EffectEngine::GetInstance()->Stop(m_handle);
		//新規再生
		m_handle = EffectEngine::GetInstance()->Play(m_effect);
		// Play直後に位置・回転・スケールを即時反映（しないと初フレームに原点で描画される）
		EffectEngine::GetInstance()->UpdateEffectWorldMatrix(
			m_handle,
			m_position,
			m_rotation,
			m_scale
		);
	}
	void Effect::Update()
	{
		EffectEngine::GetInstance()->UpdateEffectWorldMatrix(
			m_handle,
			m_position,
			m_rotation,
			m_scale
		);
	}
}