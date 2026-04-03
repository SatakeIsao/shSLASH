/**
 * Types.h
 * コリジョン関係の定義群
 */
#pragma once
#include "physics/CollisionAttr.h"
#include <cstdint>


namespace app
{
	namespace collision
	{
		struct CollisionAttribute
		{
			enum Enum : uint32_t
			{
				Ground			= 1 << 0,
				Character		= 1 << 1,
				Enemy			= 1 << 2,
			};
		};


		struct CollisionAttributeMask
		{
			enum Enum : uint32_t
			{
				Ground			= CollisionAttribute::Character,
				Character		= CollisionAttribute::Ground,
				Enemy			= 1 << 2,
				All				= 0xFFFFFFFF,
			};
		};


		namespace ghost
		{
			struct CollisionAttribute
			{
				enum Enum : uint32_t
				{
					None = 0,
					Player = 1 << 0,
					Enemy = 1 << 1,
					StaticGimmick = 1 << 2,
					Pipe = 1 << 3,
				};
			};


			struct CollisionAttributeMask
			{
				enum Enum : uint32_t
				{
					Player			= CollisionAttribute::Enemy | CollisionAttribute::Pipe,
					Enemy			= 1 << 1,
					StaticGimmick	= 1 << 2,
					Pipe			= CollisionAttribute::Player,
					All				= 0xFFFFFFFF,
				};
			};
		}
	}
}