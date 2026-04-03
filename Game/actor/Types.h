/**
 * 定義関連
 */
#pragma once
#include <cstdint>


#define appActor(name)\
public:\
	static constexpr uint32_t ID() { return Hash32(#name); }


namespace app
{
	namespace actor
	{
		enum class PlayerAnimationKind : uint8_t
		{
			Idle,
			Run,
			JumpAscend,		// 上昇
			JumpFalling,	// 落下
			JumpLand,		// 着地
			Punch,
			KnockBack,
			Dead,
			Max
		};
		enum class SlimeAnimationKind : uint8_t
		{
			Idle,
			Run,
			Attack,
			Dead,
			knockBack,
			Max
		};
	}
}