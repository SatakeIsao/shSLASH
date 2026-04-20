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
			using DeadCallback = std::function<void()>;

		private:
			DeadCallback onDead_ = nullptr;
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
			void SetOnDead(DeadCallback callback)
			{
				onDead_ = std::move(callback);
			}
		};




		/****************************************************/
		
		
		class StoneEventCharacter : public Character
		{
			appActor(StoneEventCharacter);


		private:
			using SuperClass = Character;
			using DeadCallback = std::function<void()>;

		private:
			DeadCallback onDead_ = nullptr;
			std::unique_ptr<StoneEventCharacterStateMachine> stateMachine_ = nullptr;
			std::unique_ptr<app::collision::GhostBody> ghostBody_ = nullptr;
			static int instanceCount_;
			Vector3 forward_ = g_vec3Front;
			bool isPause_ = false;


		public:
			StoneEventCharacter();
			~StoneEventCharacter();

			bool Start() override;
			void Update() override;
			void Render(RenderContext& rc) override;


		public:
			/** 初期化処理 */
			void Initialize(CharacterInitializeParameter& param) override final;


			/** 当たり判定を作り直す */
			void ResizeCollision();


			Vector3 GetForward()
			{
				return forward_;
			}

			void TakeDamage(int damegeHP)
			{
				currentHP_ -= damegeHP;
				if (currentHP_ < 0)
				{
					currentHP_ = 0;
				}
			}


			static int GetNum() {return instanceCount_;}


			template <typename TState>
			void AddState()
			{
				stateMachine_->AddState<TState>();
			}


			StoneEventCharacterStateMachine* GetStateMachine()
			{
				return stateMachine_.get();
			}

			void SetPause(bool isPause)
			{
				isPause_ = isPause;
			}
			void SetOnDead(DeadCallback callback)
			{
				onDead_ = std::move(callback);
			}
			void NotifyDead()
			{
				if (onDead_) { onDead_(); }
			}
		};




		/****************************************************/
		
		
		class MushroomEventCharacter : public Character
		{
			appActor(MushroomEventCharacter);


		private:
			using SuperClass = Character;
			using DeadCallback = std::function<void()>;

		private:
			DeadCallback onDead_ = nullptr;
			std::unique_ptr<MushroomEventCharacterStateMachine> stateMachine_ = nullptr;
			std::unique_ptr<app::collision::GhostBody> ghostBody_ = nullptr;
			static int instanceCount_;
			Vector3 forward_ = g_vec3Front;
			bool isPause_ = false;


		public:
			MushroomEventCharacter();
			~MushroomEventCharacter();


			bool Start() override;
			void Update() override;
			void Render(RenderContext& rc) override;


		public:
			/** 初期化処理 */
			void Initialize(CharacterInitializeParameter& param) override final;


			/** 当たり判定を作り直す */
			void ResizeCollision();


			Vector3 GetForward()
			{
				return forward_;
			}


			static int GetNum() { return instanceCount_; }



			template <typename TState>
			void AddState()
			{
				stateMachine_->AddState<TState>();
			}


			MushroomEventCharacterStateMachine* GetStateMachine()
			{
				return stateMachine_.get();
			}


			void SetPause(bool isPause)
			{
				isPause_ = isPause;
			}
			void SetOnDead(DeadCallback callback)
			{
				onDead_ = std::move(callback);
			}
			void NotifyDead()
			{
				if (onDead_) { onDead_(); }
			}
		};
	}
}