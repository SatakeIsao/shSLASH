/**
 * Actorファイル
 */
#include "stdafx.h"
#include "Actor.h"
#include "ActorState.h"
#include "ActorStateMachine.h"
#include "ActorStatus.h"
#include "actor/Types.h"
#include "core/ParameterManager.h"


namespace app
{
	namespace actor
	{
		IdleCharacterState::IdleCharacterState(IStateMachine* owner)
			: ICharacterState(owner)
		{
		}


		IdleCharacterState::~IdleCharacterState()
		{
		}


		void IdleCharacterState::Enter()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::Idle));
		}


		void IdleCharacterState::Update()
		{

		}


		void IdleCharacterState::Exit()
		{

		}




		/*************************************/


		RunCharacterState::RunCharacterState(IStateMachine* owner)
			: ICharacterState(owner)
		{
		}


		RunCharacterState::~RunCharacterState()
		{
		}


		void RunCharacterState::Enter()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::Run));
		}


		void RunCharacterState::Update()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			auto* characterStatus = characterStateMachine->GetStatus();
			characterStateMachine->Move(g_gameTime->GetFrameDeltaTime(), characterStatus->GetMoveSpeed());

			characterStateMachine->transform.rotation.SetRotationYFromDirectionXZ(characterStateMachine->GetMoveSpeedVector());
		}


		void RunCharacterState::Exit()
		{
		}




		/*************************************/


		AttackCharacterState::AttackCharacterState(IStateMachine* owner)
			:ICharacterState(owner)
		{
		}


		AttackCharacterState::~AttackCharacterState()
		{
		}


		void AttackCharacterState::Enter()
		{
			stateTimer_ = 0.0f;

			////ここで攻撃用のゴーストオブジェクトを作成
			attackScheduler_ = std::make_unique<app::core::TaskSchedulerSystem>();
			attackScheduler_->AddTimer(0.1f, [&]()
				{
					auto* characterStateMachine = owner_->As<CharacterStateMachine>();
					//characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::SlimeAnimationKind::Attack));
					attackBody_ = new app::collision::GhostBody();
					attackBody_->CreateSphere(characterStateMachine->GetCharacter(), characterStateMachine->GetCharacterID(), 20.0f, app::collision::ghost::CollisionAttribute::Enemy, app::collision::ghost::CollisionAttributeMask::All);
					isAttackBody_ = true;

					if (auto* eventMachine = owner_->As<app::actor::EventCharacterStateMachine>())
					{
						eventMachine->NontifyAttackGhostCreated();
					}

					// @todo for test
					const float radius = characterStateMachine->GetStatus()->GetRadius();
					
					Vector3 forward = characterStateMachine->GetMoveDirection();
					
					if (forward.LengthSq() < 0.01f) {
						forward = Vector3::Front;
					}
					attackBody_->SetPosition(characterStateMachine->transform.position + forward * (radius + radius) + Vector3(0.0f, radius, 0.0f));
			}, false);

			// DEBUG; 削除はEnterではしない
			//ゴースト削除タイマー
			attackScheduler_->AddTimer(0.1f, [&]()
				{
					if (attackBody_ != nullptr) {
						delete attackBody_;
						attackBody_ = nullptr;
						isAttackBody_ = false;
					}
				}, true);
		}


		void AttackCharacterState::Update()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			auto* characterStatus = characterStateMachine->GetStatus();

			//攻撃中も移動を続けるための処理
			//移動処理とY回転の更新
			characterStateMachine->Move(g_gameTime->GetFrameDeltaTime(), characterStatus->GetMoveSpeed());
			characterStateMachine->transform.rotation.SetRotationYFromDirectionXZ(characterStateMachine->GetMoveSpeedVector());

			//  //ゴーストの位置をスライムの現在位置に合わせて追従させる
			if (attackBody_)
			{
				const float radius = characterStateMachine->GetStatus()->GetRadius();
				Vector3 forward = characterStateMachine->GetMoveDirection();
				if (forward.LengthSq() <= 0.01f)
				{
					forward = Vector3::Front;
				}
				attackBody_->SetPosition(characterStateMachine->transform.position + forward * (radius + radius) + Vector3(0.0f, radius, 0.0f));
			}

			stateTimer_ += g_gameTime->GetFrameDeltaTime();
			if (attackScheduler_)
			{
				attackScheduler_->Update(g_gameTime->GetFrameDeltaTime());
			}
		}


		void AttackCharacterState::Exit()
		{
			attackScheduler_.reset(nullptr);

			if (attackBody_ != nullptr) 
			{
				delete attackBody_;
				attackBody_ = nullptr;
				isAttackBody_ = false;
			}
		}


		bool AttackCharacterState::CanChangeState() const
		{
			/** TODO; ある程度の距離外になったら　　アニメーション再生は廃止したいな
			     あくまで攻撃ステートは攻撃用のゴーストオブジェクトを付与してるだけ
				 ゴーストの付与の切り替えかな
			 */


			return stateTimer_ >3.0f;
			
			//auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			//auto* modelRender = characterStateMachine->GetModelRender();
			//return !modelRender->IsPlayingAnimation();
		}


		/*************************************/


		JumpCharacterState::JumpCharacterState(IStateMachine* owner)
			: ICharacterState(owner)
		{
		}


		JumpCharacterState::~JumpCharacterState()
		{
		}


		void JumpCharacterState::Enter()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			auto* characterStatus = characterStateMachine->GetStatus();

			characterStateMachine->Jump(characterStatus->GetJumpPower());

			characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpAscend));

			characterStateMachine->GetModelRender()->SetAnimationSpeed(2.5f);
		}


		void JumpCharacterState::Update()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();

			switch (jumpPhase_)
			{
			case JumpPhase::Ascend:
			{
				// 上昇が終わったら落下フェーズへ
				if (characterStateMachine->GetCharacterController()->GetVerticalVelocity() < 0.0f) {
					characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpFalling));
					jumpPhase_ = JumpPhase::Falling;
				}
				break;
			}
			case JumpPhase::Falling:
			{
				// 地面に着地したら着地フェーズへ
				if (characterStateMachine->GetCharacterController()->IsOnGround()) {
					characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpLand));
					jumpPhase_ = JumpPhase::Land;
				}
				break;
			}
			case JumpPhase::Land:
			{
				break;
			}
			}

			auto* characterStatus = characterStateMachine->GetStatus();
			characterStateMachine->Move(g_gameTime->GetFrameDeltaTime(), characterStatus->GetJumpMoveSpeed());
			characterStateMachine->transform.rotation.SetRotationYFromDirectionXZ(characterStateMachine->GetMoveSpeedVector());
		}


		void JumpCharacterState::Exit()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.0f);
		}


		bool JumpCharacterState::CanChangeState() const
		{
			if (jumpPhase_ != JumpPhase::Land) {
				return false;
			}
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			if (!characterStateMachine->GetCharacterController()->IsOnGround()) {
				return false;
			}
			if (characterStateMachine->GetModelRender()->IsPlayingAnimation()) {
				return false;
			}
			return true;
		}




		/*************************************/


		FallingCharacterState::FallingCharacterState(IStateMachine* owner)
			: ICharacterState(owner)
		{
		}


		FallingCharacterState::~FallingCharacterState()
		{
		}


		void FallingCharacterState::Enter()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpFalling));
		}


		void FallingCharacterState::Update()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			auto* characterStatus = characterStateMachine->GetStatus();
			characterStateMachine->Move(g_gameTime->GetFrameDeltaTime(), characterStatus->GetJumpMoveSpeed());
			characterStateMachine->transform.rotation.SetRotationYFromDirectionXZ(characterStateMachine->GetMoveSpeedVector());
		}


		void FallingCharacterState::Exit()
		{
		}




		/*************************************/


		PunchCharacterState::PunchCharacterState(IStateMachine* owner)
			: ICharacterState(owner)
		{
		}


		PunchCharacterState::~PunchCharacterState()
		{
		}


		void PunchCharacterState::Enter()
		{
			attackScheduler_ = std::make_unique<app::core::TaskSchedulerSystem>();
			attackScheduler_->AddTimer(0.1f, [&]()
				{
					auto* characterStateMachine = owner_->As<CharacterStateMachine>();
					characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::Punch));
					attackBody_ = new app::collision::GhostBody();
					attackBody_->CreateSphere(characterStateMachine->GetCharacter(), characterStateMachine->GetCharacterID(), 20.0f, app::collision::ghost::CollisionAttribute::Player, app::collision::ghost::CollisionAttributeMask::All);
					// @todo for test
					const float radius = characterStateMachine->GetStatus()->GetRadius();
					attackBody_->SetPosition(characterStateMachine->transform.position + characterStateMachine->GetMoveDirection() * (radius + radius) + Vector3(0.0f, radius, 0.0f));
				}, false);
			attackScheduler_->AddTimer(0.1f, [&]()
				{
					delete attackBody_;
					attackBody_ = nullptr;
				}, true);
		}


		void PunchCharacterState::Update()
		{
			attackScheduler_->Update(g_gameTime->GetFrameDeltaTime());
		}


		void PunchCharacterState::Exit()
		{
			attackScheduler_.reset(nullptr);
			if (attackBody_ != nullptr) 
			{
				delete attackBody_;
				attackBody_ = nullptr;
			}
		}


		bool PunchCharacterState::CanChangeState() const
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			auto* modelRender = characterStateMachine->GetModelRender();
			return !modelRender->IsPlayingAnimation();
		}




		/*************************************/


		WarpInCharacterState::WarpInCharacterState(IStateMachine* owner)
			: ICharacterState(owner)
		{
		}


		WarpInCharacterState::~WarpInCharacterState()
		{
		}


		void WarpInCharacterState::Enter()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpFalling));
			characterStateMachine->SetInputPower(0.0f);
			characterStateMachine->ClearMomveSpeedVector();
			auto* characterStatus = characterStateMachine->GetStatus();
			scaleCurve_.Initialize(characterStatus->GetWarpStartScale(), characterStatus->GetWarpEndScale(), characterStatus->GetWarpTimeSeconds(), app::util::EasingType::Linear);
			scaleCurve_.Play();
			translateCurve_.Initialize(characterStateMachine->transform.position, characterStateMachine->GetWarpStartPosition(), characterStatus->GetWarpTimeSeconds() * 0.3f, app::util::EasingType::Linear);
			translateCurve_.Play();
		}


		void WarpInCharacterState::Update()
		{
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			scaleCurve_.Update(deltaTime);
			translateCurve_.Update(deltaTime);

			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->GetCharacterController()->RequestTeleport();
			characterStateMachine->transform.scale = Vector3(scaleCurve_.GetCurrentValue());
			Vector3 newPosition = translateCurve_.GetCurrentValue();
			characterStateMachine->transform.position.x = newPosition.x;
			characterStateMachine->transform.position.y -= 1.0f; // NOTE: 下に埋め込みたいので
			characterStateMachine->transform.position.z = newPosition.z;
		}


		void WarpInCharacterState::Exit()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->GetCharacterController()->RequestTeleport();
			characterStateMachine->transform.position = characterStateMachine->GetWarpEndPosition();
		}


		bool WarpInCharacterState::CanChangeState() const
		{
			if (scaleCurve_.IsPlaying()) {
				return false;
			}
			return true;
		}




		/*************************************/


		WarpOutCharacterState::WarpOutCharacterState(IStateMachine* owner)
			: ICharacterState(owner)
		{
		}


		WarpOutCharacterState::~WarpOutCharacterState()
		{
		}


		void WarpOutCharacterState::Enter()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::Idle));
			auto* characterStatus = characterStateMachine->GetStatus();
			scaleCurve_.Initialize(characterStatus->GetWarpEndScale(), characterStatus->GetWarpStartScale(), characterStatus->GetWarpTimeSeconds(), app::util::EasingType::Linear);
			scaleCurve_.Play();
		}


		void WarpOutCharacterState::Update()
		{
			scaleCurve_.Update(g_gameTime->GetFrameDeltaTime());

			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->GetCharacterController()->RequestTeleport();

			characterStateMachine->transform.scale = Vector3(scaleCurve_.GetCurrentValue());
		}


		void WarpOutCharacterState::Exit()
		{
		}


		bool WarpOutCharacterState::CanChangeState() const
		{
			if (scaleCurve_.IsPlaying()) {
				return false;
			}
			return true;
		}




		/*************************************/


		DeadCharacterState::DeadCharacterState(IStateMachine* owner)
			:ICharacterState(owner)
		{
		}


		DeadCharacterState::~DeadCharacterState()
		{
		}


		void DeadCharacterState::Enter()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			//キャラクター固有の死亡処理を実行
			characterStateMachine->OnEnterDead();

			timer_ = 0.0f;
		}


		void DeadCharacterState::Update()
		{
			timer_ += g_gameTime->GetFrameDeltaTime();
		}


		void DeadCharacterState::Exit()
		{ 
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			// DEBUG_TEST: キャラクター固有の志望解除のを実行
			characterStateMachine->OnExitDead();
		}


		bool DeadCharacterState::CanChangeState() const
		{
			return timer_ > 2.0f;
		}

		


		/*************************************/


		KnockBackCharacterState::KnockBackCharacterState(IStateMachine* owner)
			: ICharacterState(owner)
		{
		}


		KnockBackCharacterState::~KnockBackCharacterState()
		{
		}


		void KnockBackCharacterState::Enter()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->OnEnterKnockBack();
		}
		

		void KnockBackCharacterState::Update()
		{
			timer_ += g_gameTime->GetFrameDeltaTime();
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();

			bool isLanded = false;

			if (timer_ > 0.1f)
			{
				isLanded = characterStateMachine->GetCharacterController()->IsOnGround();
			}

			if (!isLanded)
			{
				//時間経過で徐々に減衰させる
				float deceleration = 1.0f - timer_;
				if (deceleration < 0.0f) {
					deceleration = 0.0f;
				}
				//スピード調整
				float currentSpeed = 500.0f * deceleration;

				characterStateMachine->Move(g_gameTime->GetFrameDeltaTime(), currentSpeed);
			}
		}


		void KnockBackCharacterState::Exit()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->OnExitKnockBack();
		}


		bool KnockBackCharacterState::CanChangeState() const
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();

			bool isAnimFinished = !characterStateMachine->GetModelRender()->IsPlayingAnimation();
			bool isLanded = false;

			if (timer_ > 0.1f)
			{
				isLanded = characterStateMachine->GetCharacterController()->IsOnGround();
			}

			return ((isAnimFinished && isLanded) || timer_ > 2.0f);
		}
	}
}