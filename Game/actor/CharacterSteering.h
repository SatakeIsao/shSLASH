/**
 * CharacterSteeringファイル
 */
#pragma once


namespace app
{
	namespace actor
	{
		class Character;


		class CharacterSteering : public Noncopyable
		{
		private:
			uint8_t padIndex_ = 0;

			Character* target_ = nullptr;



		public:
			CharacterSteering() {}
			virtual ~CharacterSteering() {}

			void Initialize(Character* target, const uint8_t index);
			void Update();


		private:
			inline GamePad* GetPad() { return g_pad[padIndex_]; }
		};
	}
}