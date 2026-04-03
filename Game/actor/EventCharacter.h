/**
 * EventCharacterファイル
 */
#pragma once
#include "Actor.h"
#include "ActorStateMachine.h"
#include "actor/Types.h"


namespace app
{
	namespace actor
	{
		class EventCharacter : public Character
		{
			appActor(EventCharacter);

		private:
			using SuperClass = Character;

		private:
			std::unique_ptr<EventCharacterStateMachine> stateMachine_ = nullptr;
			std::unique_ptr<app::collision::GhostBody> ghostBody_ = nullptr;
			Vector3 forward_ = g_vec3Front;
			bool isPause_ = false;

		public:
			EventCharacter();
			~EventCharacter();

			bool Start() override;
			void Update() override;
			void Render(RenderContext& rc) override;


		public:
			void Initialize(CharacterInitializeParameter& param) override final;
			/** 当たり判定を作り直す */
			void ResizeCollision();
			Vector3 GetForward()
			{
				return forward_;
			}

			template <typename TState>
			void AddState() 
			{
				stateMachine_->AddState<TState>();
			}

			EventCharacterStateMachine* GetStateMachine()
			{
				return stateMachine_.get();
			}

			void SetPause(bool isPause)
			{
				isPause_ = isPause;
			}
		};
	}
}