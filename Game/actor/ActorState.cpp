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
#include "sound/SoundManager.h"


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
			characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::Idle), 0.2f);
			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.0f);
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
			characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::Run), 0.1f);
			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.0f);
			/** 走り開始の固有処理を委譲 */
			characterStateMachine->OnEnterRun();
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
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->OnExitRun();
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
					characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::SlimeAnimationKind::Attack));
					characterStateMachine->GetModelRender()->SetAnimationSpeed(1.0f);
					attackBody_ = new app::collision::GhostBody();
					attackBody_->CreateSphere(characterStateMachine->GetCharacter(), characterStateMachine->GetCharacterID(), 20.0f, app::collision::ghost::CollisionAttribute::Enemy, app::collision::ghost::CollisionAttributeMask::All);
					isAttackBody_ = true;

					//if (auto* eventMachine = owner_->As<app::actor::EventCharacterStateMachine>())
					//{
					//	eventMachine->NontifyAttackGhostCreated();
					//}
					if (auto* m = owner_->As<app::actor::StoneEventCharacterStateMachine>())
					{
						m->NontifyAttackGhostCreated();
					}
					else if (auto* m = owner_->As<app::actor::MushroomEventCharacterStateMachine>())
					{
						m->NontifyAttackGhostCreated();
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


			return stateTimer_ >1.3f;

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

			//characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpAscend));

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
					//characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpFalling));
					jumpPhase_ = JumpPhase::Falling;
				}
				break;
			}
			case JumpPhase::Falling:
			{
				// 地面に着地したら着地フェーズへ
				if (characterStateMachine->GetCharacterController()->IsOnGround()) {
					//characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpLand));
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
			//characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpFalling));
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


		void ComboAttackCharacterState::Enter()
		{
			stateTimer_ = 0.0f;
			isComboInput_ = false;
			isFirstFrame_ = true;

			const auto param = GetComboParam();
			auto* csm = owner_->As<CharacterStateMachine>();
			csm->GetModelRender()->PlayAnimation(static_cast<uint8_t>(param.animKind), 0.5f);
			csm->GetModelRender()->SetAnimationSpeed(param.animSpeed);
			app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::AtkWeak), false);

			attackScheduler_ = std::make_unique<app::core::TaskSchedulerSystem>();
			attackScheduler_->AddTimer(param.attackBodyDelay, [&]()
				{
					const auto p = GetComboParam();
					auto* csm = owner_->As<CharacterStateMachine>();
					attackBody_ = new app::collision::GhostBody();
					attackBody_->CreateSphere(
						csm->GetCharacter(), csm->GetCharacterID(),
						p.attackBodyRadius,
						app::collision::ghost::CollisionAttribute::Player,
						app::collision::ghost::CollisionAttributeMask::All);
					const float radius = csm->GetStatus()->GetRadius();
					attackBody_->SetPosition(
						csm->transform.position
						+ csm->GetMoveDirection() * (radius + radius)
						+ Vector3(0.0f, radius, 0.0f));
				}, false);
			attackScheduler_->AddTimer(param.attackBodyDuration, [&]()
				{
					delete attackBody_;
					attackBody_ = nullptr;
				}, true);
		}


		void ComboAttackCharacterState::Update()
		{
			stateTimer_ += g_gameTime->GetFrameDeltaTime();
			attackScheduler_->Update(g_gameTime->GetFrameDeltaTime());

			// Enter と同じフレームは入力を無視
			if (isFirstFrame_)
			{
				isFirstFrame_ = false;
			}
			else
			{
				if (g_pad[0]->IsTrigger(enButtonB))
				{
					isComboInput_ = true;
				}
			}

			// ゴースト追従
			if (attackBody_)
			{
				auto* csm = owner_->As<CharacterStateMachine>();
				const float radius = csm->GetStatus()->GetRadius();
				Vector3 forward = csm->GetMoveDirection();
				if (forward.LengthSq() <= 0.01f) forward = Vector3::Front;
				attackBody_->SetPosition(
					csm->transform.position
					+ forward * (radius + radius)
					+ Vector3(0.0f, radius, 0.0f));
			}
		}


		void ComboAttackCharacterState::Exit()
		{
			attackScheduler_.reset(nullptr);
			if (attackBody_ != nullptr)
			{
				delete attackBody_;
				attackBody_ = nullptr;
			}
		}


		bool ComboAttackCharacterState::CanChangeState() const
		{
			const auto param = GetComboParam();
			if (isComboInput_)
			{
				// コンボ入力あり タイマーで早めに遷移許可
				return stateTimer_ >= param.comboWindowTime;
			}
			// コンボ入力なし アニメ終了で遷移許可
			auto* csm = owner_->As<CharacterStateMachine>();
			return !csm->GetModelRender()->IsPlayingAnimation();
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
			//characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpFalling));
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
			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.0f);

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
			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.0f);
			
			if (auto* battleMachine = owner_->As<BattleCharacterStateMachine>()) {
				battleMachine->CheckAndConsumeKnockBack();
			}
			timer_ = 0.0f;
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
				float currentSpeed = 300.0f * deceleration;

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
		



		/*************************************/


		GuardCharacterState::GuardCharacterState(IStateMachine* owner)
			: ICharacterState(owner)
		{}


		GuardCharacterState::~GuardCharacterState()
		{}


		void GuardCharacterState::Enter()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->OnEnterGuard();
			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.0f);
			/** ガード開始 */
			characterStateMachine->SetGuarding(true);

			if (auto* battleMachine = owner_->As<BattleCharacterStateMachine>()) {
				battleMachine->RequestGuardEffect();
			}
		}


		void GuardCharacterState::Update()
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


		void GuardCharacterState::Exit()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			/** ガード終了 */
			characterStateMachine->SetGuarding(false);
			characterStateMachine->OnExitGuard();
		}


		bool GuardCharacterState::CanChangeState() const
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


		

		/*************************************/


		ChargeAttackCharacterState::ChargeAttackCharacterState(IStateMachine* owner)
			: ICharacterState(owner)
		{}


		ChargeAttackCharacterState::~ChargeAttackCharacterState()
		{}


		void ChargeAttackCharacterState::Enter()
		{
			chargeAttackPhase_ = ChargeAttackPhase::Start;

			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			auto* characterStatus = characterStateMachine->GetStatus();


			// BattleCharacterStateMachineなら、チャージエフェクトの再生リクエストを出す
			if (auto* battleMachine = owner_->As<BattleCharacterStateMachine>()) {
				battleMachine->RequestChargeEffect();
			}
			characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::ChargedAttackStart), 0.1f);
			app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Charging));

			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.0f);
		}


		void ChargeAttackCharacterState::Update()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();

			switch (chargeAttackPhase_)
			{
			case ChargeAttackPhase::Start:
			{
				// 上昇が終わったら落下フェーズへ
				if (!characterStateMachine->GetModelRender()->IsPlayingAnimation()) {
					characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::ChargedAttackLooping), 0.1f);
					chargeAttackPhase_ = ChargeAttackPhase::Looping;
				}
					
				break;
			}
			case ChargeAttackPhase::Looping:
			{
				// 地面に着地したら着地フェーズへ
				if (!characterStateMachine->IsPressA()) {
					characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::ChargedAttackEnd), 0.1f);
					app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::AtkCharge));
					chargeAttackPhase_ = ChargeAttackPhase::End;
					// BattleCharacterStateMachineなら、チャージエフェクトの再生リクエストを出す
					if (auto* battleMachine = owner_->As<BattleCharacterStateMachine>()) {
						battleMachine->RequestChargeAttackEffect();
					}

					// フェーズ遷移の瞬間に一度だけ生成
					attackScheduler_ = std::make_unique<app::core::TaskSchedulerSystem>();
					attackScheduler_->AddTimer(0.1f, [&]()
						{
							auto* csm = owner_->As<CharacterStateMachine>();
							attackBody_ = new app::collision::GhostBody();
							attackBody_->CreateSphere(
								csm->GetCharacter(),
								csm->GetCharacterID(),
								25.0f,
								app::collision::ghost::CollisionAttribute::Player,
								app::collision::ghost::CollisionAttributeMask::All);
							const float radius = csm->GetStatus()->GetRadius();
							attackBody_->SetPosition(
								csm->transform.position
								+ csm->GetMoveDirection() * (radius + radius)
								+ Vector3(0.0f, radius, 0.0f));
						}, false);
					attackScheduler_->AddTimer(0.1f, [&]()
						{
							delete attackBody_;
							attackBody_ = nullptr;
						}, true);
				}
				break;
			}
			case ChargeAttackPhase::End:
			{
				// スケジューラーの更新だけ行う（生成は Looping → End の遷移時に済んでいる）
				if (attackScheduler_) {
					attackScheduler_->Update(g_gameTime->GetFrameDeltaTime());
				}
				break;
			}
		}

			auto* characterStatus = characterStateMachine->GetStatus();
		}


		void ChargeAttackCharacterState::Exit()
		{
			attackScheduler_.reset(nullptr);  // スケジューラーを破棄

			// ゴーストボディが残っていたら確実に削除
			if (attackBody_ != nullptr) {
				delete attackBody_;
				attackBody_ = nullptr;
			}

			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.0f);
		}


		bool ChargeAttackCharacterState::CanChangeState() const
		{
			if (chargeAttackPhase_ != ChargeAttackPhase::End) {
				return false;
			}
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			
			if (characterStateMachine->GetModelRender()->IsPlayingAnimation()) {
				return false;
			}
			return true;
		}




		/*************************************/


		AvoidanceCharacterState::AvoidanceCharacterState(IStateMachine* owner)
			: ICharacterState(owner)
		{}


		AvoidanceCharacterState::~AvoidanceCharacterState()
		{}


		void AvoidanceCharacterState::Enter()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->OnEnterAvoidance();
			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.5f);

			// 回避開始時のスティックの入力を保存
			avoidanceDirection_ = characterStateMachine->GetMoveDirection();

			timer_ = 0.0f;

			// 回避中のフラグを立てる
			characterStateMachine->SetAvoiding(true);
		}


		void AvoidanceCharacterState::Update()
		{
			timer_ += g_gameTime->GetFrameDeltaTime();
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();

			// 回避の強さと持続時間
			const float duration = 1.0f;
			const float speed = 500.0f;

			if (timer_ < duration)
			{
				// 徐々に減速させる計算
				float deceleration = 1.0f - (timer_ / duration);
				float currentSpeed = speed * deceleration;

				// 保存した入力をセットし、移動を実行
				characterStateMachine->SetMoveDirection(avoidanceDirection_);
				characterStateMachine->SetInputPower(1.0f);
				characterStateMachine->Move(g_gameTime->GetFrameDeltaTime(), currentSpeed);

				// Move内で計算された「実際の移動ベクトル」を取得
				auto speedVec = characterStateMachine->GetMoveSpeedVector();

				// その移動方向へ振り向かせる
				if (speedVec.LengthSq() > 0.01f)
				{
					characterStateMachine->transform.rotation.SetRotationYFromDirectionXZ(speedVec);
				}
			}
		}


		void AvoidanceCharacterState::Exit()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			// 回避終了
			characterStateMachine->SetAvoiding(false);
			characterStateMachine->OnExitAvoidance();
		}

		bool AvoidanceCharacterState::CanChangeState() const
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			auto* modelRender = characterStateMachine->GetModelRender();
			return !modelRender->IsPlayingAnimation();
		}




		/*************************************/


		InjuredIdleCharacterState::InjuredIdleCharacterState(IStateMachine* owner)
			: ICharacterState(owner)
		{
		}


		InjuredIdleCharacterState::~InjuredIdleCharacterState()
		{
		}


		void InjuredIdleCharacterState::Enter()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::InjuredIdle), 0.2);
			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.0f);
			app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::HpDanger), false);
		}


		void InjuredIdleCharacterState::Update()
		{
		}


		void InjuredIdleCharacterState::Exit()
		{
		}

		bool InjuredIdleCharacterState::CanChangeState() const
		{
			return true;
		}




		/*************************************/


		InjuredRunCharacterState::InjuredRunCharacterState(IStateMachine* owner)
			: ICharacterState(owner)
		{}


		InjuredRunCharacterState::~InjuredRunCharacterState()
		{}


		void InjuredRunCharacterState::Enter()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::InjuredRun), 0.2f);
			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.0f);
			characterStateMachine->OnEnterInjuredRun();
		}


		void InjuredRunCharacterState::Update()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			auto* characterStatus = characterStateMachine->GetStatus();
			characterStateMachine->Move(g_gameTime->GetFrameDeltaTime(), characterStatus->GetMoveSpeed()* 0.5f);

			characterStateMachine->transform.rotation.SetRotationYFromDirectionXZ(characterStateMachine->GetMoveSpeedVector());
		}


		void InjuredRunCharacterState::Exit()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->OnExitInjuredRun();
		}

		bool InjuredRunCharacterState::CanChangeState() const
		{
			return true;
		}




		/*************************************/


		KipUpCharacterState::KipUpCharacterState(IStateMachine* owner)
			: ICharacterState(owner)
		{
		}


		KipUpCharacterState::~KipUpCharacterState()
		{
		}


		void KipUpCharacterState::Enter()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::KipUp), 0.1f);
			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.0f);

			seTimer_ = 0.0f;
			sePlayed_ = false;
		}


		void KipUpCharacterState::Update()
		{
			static constexpr float SE_DELAY = 1.0f;

			if (!sePlayed_)
			{
				seTimer_ += g_gameTime->GetFrameDeltaTime();
				if (seTimer_ >= SE_DELAY)
				{
					app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::KipUp), false);
					sePlayed_ = true;
				}
			}
		}


		void KipUpCharacterState::Exit()
		{}

		bool KipUpCharacterState::CanChangeState() const
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			// アニメーションが再生中なら遷移させない
			return !characterStateMachine->GetModelRender()->IsPlayingAnimation();
		}




		/*************************************/

		PatrolCharacterState::PatrolCharacterState(IStateMachine* owner)
			: ICharacterState(owner)
		{
			auto seed = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(this)) ^ time(nullptr);
			randomEngine_.seed(seed);
		}


		PatrolCharacterState::~PatrolCharacterState()
		{
		}


		void PatrolCharacterState::Enter()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();

			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.0f);

			/** uniform_real_distribution：確率に偏りをなくすために使用 */
			std::uniform_real_distribution<float> timeDist(1.0f, 3.0f);
			patrolTimer_ = timeDist(randomEngine_);

			std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
			float angle = angleDist(randomEngine_);

			float rad = angle * (3.14159265f / 180.0f);
			patrolDirection_ = Vector3(std::sinf(rad), 0.0f, std::cosf(rad));
			patrolDirection_.Normalize();

			characterStateMachine->SetMoveDirection(patrolDirection_);

			timer_ = 0.0f;

		}


		void PatrolCharacterState::Update()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			auto* characterStatus = characterStateMachine->GetStatus();

			characterStateMachine->Move(g_gameTime->GetFrameDeltaTime(), characterStatus->GetMoveSpeed());

			auto speedVec = characterStateMachine->GetMoveSpeedVector();
			if(speedVec.Length() > 0.01f)
			{
				characterStateMachine->transform.rotation.SetRotationYFromDirectionXZ(speedVec);
			}

			timer_ += g_gameTime->GetFrameDeltaTime();
		}


		void PatrolCharacterState::Exit()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->SetMoveDirection(Vector3::Zero);
		}


		bool PatrolCharacterState::CanChangeState() const
		{
			return timer_ >= patrolTimer_;
		}
}
}