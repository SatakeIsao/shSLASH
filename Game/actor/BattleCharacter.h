/**
 * BattleCharacterファイル
 */
#pragma once
#include "Actor.h"
#include "ActorStateMachine.h"
#include "actor/Types.h"


namespace app
{
	namespace actor
	{
		class BattleCharacter : public Character
		{
			appActor(BattleCharacter);


		private:
			using SuperClass = Character;


		private:
			std::unique_ptr<BattleCharacterStateMachine> stateMachine_ = nullptr;
			std::unique_ptr<app::collision::GhostBody> ghostBody_ = nullptr;

			/** TODO: jsonで管理 */
			int currentHP_ = 8;
			bool isPause_ = false;

		public:
			BattleCharacter();
			~BattleCharacter();

			bool Start() override;
			void Update() override;
			void Render(RenderContext& rc) override;


		public:
			void Initialize(CharacterInitializeParameter& param) override final;


		public:
			template <typename TState>
			void AddState()
			{
				stateMachine_->AddState<TState>();
			}


			BattleCharacterStateMachine* GetStateMachine()
			{
				return stateMachine_.get();
			}


			app::collision::GhostBody* GetGhostBody() const
			{
				return ghostBody_.get();
			}


			int GetCurrentHP() const
			{
				return currentHP_;
			}


			void TakeDamage(int damegeHP)
			{
				currentHP_ -= damegeHP;
				if (currentHP_ < 0)
				{
					currentHP_ = 0;
				}
			}


			void SetPouse(bool isPause)
			{
				isPause_ = isPause;
			}
		};
	}
}