/**
 * ActorState.h
 * キャラクターステート関連
 */
#pragma once


/**
 * 当初はenumで管理しようと思ったが、
 * ハッシュ値に変更
 * 理由：単純に文字列で検索をかけるより、
 * 数値という名のIDで検索かけた方が早いから。
 */
#define appState(name)\
public:\
	static constexpr uint32_t ID() { return Hash32(#name); }



namespace app
{
	namespace collision
	{
		class GhostBody;
	}


	namespace actor
	{
		class IStateMachine;


		class ICharacterState : public Noncopyable
		{
		protected:
			IStateMachine* owner_ = nullptr;


		public:
			ICharacterState(IStateMachine* owner) : owner_(owner) {}
			virtual ~ICharacterState() {}

			virtual void Enter() = 0;
			virtual void Update() = 0;
			virtual void Exit() = 0;

			virtual bool CanChangeState() const { return false; }
		};




		class IdleCharacterState : public ICharacterState
		{
			appState(IdleCharacterState);


		public:
			IdleCharacterState(IStateMachine* owner);
			~IdleCharacterState();

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		class RunCharacterState : public ICharacterState
		{
			appState(RunCharacterState);


		public:
			RunCharacterState(IStateMachine* owner);
			~RunCharacterState();

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		class AttackCharacterState :public ICharacterState
		{
			appState(AttackCharacterState);


		private:
			app::collision::GhostBody* attackBody_ = nullptr;
			std::unique_ptr<app::core::TaskSchedulerSystem> attackScheduler_;
			float stateTimer_ = 0.0f;
			bool isAttackBody_ = false;

		public:
			AttackCharacterState(IStateMachine* owner);
			~AttackCharacterState();

			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;

			bool IsAttackBody() const
			{
				return isAttackBody_;
			}
		};




		class JumpCharacterState : public ICharacterState
		{
			appState(JumpCharacterState);


		private:
			enum class JumpPhase
			{
				Ascend,		// 上昇
				Falling,	// 落下
				Land		// 着地
			};


		private:
			JumpPhase jumpPhase_ = JumpPhase::Ascend;


		public:
			JumpCharacterState(IStateMachine* owner);
			~JumpCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;
		};




		class FallingCharacterState : public ICharacterState
		{
			appState(FallingCharacterState);


		public:
			FallingCharacterState(IStateMachine* owner);
			~FallingCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		class PunchCharacterState : public ICharacterState
		{
			appState(PunchCharacterState);


		private:
			app::collision::GhostBody* attackBody_ = nullptr;
			std::unique_ptr<app::core::TaskSchedulerSystem> attackScheduler_;


		public:
			PunchCharacterState(IStateMachine* owner);
			~PunchCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;
		};




		class WarpInCharacterState : public ICharacterState
		{
			appState(WarpCharacterState);


		private:
			app::util::Vector3Curve translateCurve_;
			app::util::FloatCurve scaleCurve_;


		public:
			WarpInCharacterState(IStateMachine* owner);
			~WarpInCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;
		};




		class WarpOutCharacterState : public ICharacterState
		{
			appState(WarpOutCharacterState);


		private:
			app::util::FloatCurve scaleCurve_;


		public:
			WarpOutCharacterState(IStateMachine* owner);
			~WarpOutCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;
		};




		class DeadCharacterState : public ICharacterState
		{
			appState(DeadCharacterState);


		private:
			float timer_ = 0.0f;

		public:
			DeadCharacterState(IStateMachine* owner);
			~DeadCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;
		};




		class KnockBackCharacterState : public ICharacterState
		{
			appState(KnockBackCharacterState);


		private:
			float timer_ = 0.0f;

		public:
			KnockBackCharacterState(IStateMachine* owner);
			~KnockBackCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;
		};
	}
}