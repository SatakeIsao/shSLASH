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
#include "effect/SwordDecalManager.h"
#include "battle/BattleManager.h"
#include "camera/CameraManager.h"


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

			auto* characterStateMachine = owner_->As<CharacterStateMachine>();

			// アニメーション即時再生（各エネミーの OnEnterAttack に委譲）
			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.0f);
			characterStateMachine->OnEnterAttack();

			// ゴーストボディ生成タイミングはエネミーごとに GetGhostBodyDelay() で決まる
			const float ghostDelay = characterStateMachine->GetGhostBodyDelay();

			attackScheduler_ = std::make_unique<app::core::TaskSchedulerSystem>();
			attackScheduler_->AddTimer(ghostDelay, [&]()
				{
					auto* characterStateMachine = owner_->As<CharacterStateMachine>();

					// 既にアタックゴーストがある場合は解放してから再生成
					if (attackBody_ != nullptr) {
						delete attackBody_;
						attackBody_ = nullptr;
					}
					attackBody_ = new app::collision::GhostBody();
					attackBody_->CreateSphere(characterStateMachine->GetCharacter(), characterStateMachine->GetCharacterID(), 20.0f, app::collision::ghost::CollisionAttribute::Enemy, app::collision::ghost::CollisionAttributeMask::All);
					isAttackBody_ = true;

					const float radius = characterStateMachine->GetStatus()->GetRadius();
					Vector3 forward = characterStateMachine->GetMoveDirection();
					if (forward.LengthSq() < 0.01f) {
						forward = Vector3::Front;
					}
					attackBody_->SetPosition(characterStateMachine->transform.position + forward * (radius + radius) + Vector3(0.0f, radius, 0.0f));
				}, false);

			// ゴースト削除タイマー
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
			CharacterStateMachine* sm = owner_->As<CharacterStateMachine>();
			sm->OnExitAttack();

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


			return stateTimer_ > 1.3f;

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

					// 常にキャラクターの向きを使用
					Vector3 forward = Vector3::Front;
					csm->transform.rotation.Apply(forward);

					Vector3 attackPos =
						csm->transform.position
						+ forward * (radius * 4)
						+ Vector3(0.0f, radius, 0.0f);

					attackBody_->SetPosition(attackPos);

					/** 剣痕デカール: 壁のみ */
					if (app::effect::SwordDecalManager::IsAvailable())
					{
						Vector3 wallRayStart = csm->transform.position + Vector3(0.0f, radius, 0.0f);
						Vector3 wallRayEnd   = attackPos + forward * (radius * 1.0f);
						RaycastHit hit{};
						auto wallOnly = [](const btCollisionObject& obj) {
							return obj.getUserIndex() != enCollisionAttr_Character;
						};
						if (PhysicsWorld::Get().Raycast(wallRayStart, wallRayEnd, hit, ALL_COLLISION_ATTRIBUTE_MASK, wallOnly))
							if (fabsf(hit.normal.y) < 0.7f)
								app::effect::SwordDecalManager::Get().SpawnDecal(
									hit.point, hit.normal, forward);
					}
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
				// 常にキャラクターの向きを使用
				forward = Vector3::Front;
				csm->transform.rotation.Apply(forward);

				attackBody_->SetPosition(
					csm->transform.position
					+ forward * (radius * 4)
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


		void SlashThirdCharacterState::Exit()
		{
			ComboAttackCharacterState::Exit();
			if (app::battle::BattleManager::IsAvailable())
				app::battle::BattleManager::Get().NotifyComboAttackCompleted();
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

			// OnEnterKnockBack の中で isBlowBack_ がリセットされるため、先にフラグを取得する
			bool isBlowBack = false;
			if (auto* stone = owner_->As<StoneEventCharacterStateMachine>())
			{
				isBlowBack = stone->IsBlowBack();
			}
			else if (auto* mushroom = owner_->As<MushroomEventCharacterStateMachine>())
			{
				isBlowBack = mushroom->IsBlowBack();
			}

			characterStateMachine->OnEnterKnockBack();
			characterStateMachine->GetModelRender()->SetAnimationSpeed(0.8f);

			// 溜め攻撃ヒット時：ヒットストップ＋振動を開始
			// Jump はヒットストップ終了後に呼ぶので、ここでは予約だけ
			if (isBlowBack)
			{
				int chargeLevel = 0;
				hitStopDuration_ = 0.0f;

				if (auto* stone = owner_->As<StoneEventCharacterStateMachine>())
				{
					chargeLevel = stone->GetKnockBackChargeLevel();
					if (const auto* p = app::core::ParameterManager::Get().GetParameter<app::core::MasterStoneEventCharacterParameter>())
					{
						hitStopDuration_ = chargeLevel <= 0 ? p->hitStopDurationSmall
						                 : chargeLevel == 1 ? p->hitStopDurationMedium
						                                    : p->hitStopDurationLarge;
					}
				}
				else if (auto* mushroom = owner_->As<MushroomEventCharacterStateMachine>())
				{
					chargeLevel = mushroom->GetKnockBackChargeLevel();
					if (const auto* p = app::core::ParameterManager::Get().GetParameter<app::core::MasterMushroomEventCharacterParameter>())
					{
						hitStopDuration_ = chargeLevel <= 0 ? p->hitStopDurationSmall
						                 : chargeLevel == 1 ? p->hitStopDurationMedium
						                                    : p->hitStopDurationLarge;
					}
				}
				hitStopTimer_    = hitStopDuration_;
				vibrationElapsed_ = 0.0f;
				jumpPending_ = true;
				// ノックバック方向の XZ 平面 90° 回転（横揺れ）を振動軸にする
				const Vector3 kbDir = characterStateMachine->GetMoveDirection();
				vibrationAxis_ = Vector3(-kbDir.z, 0.0f, kbDir.x);
				characterStateMachine->GetModelRender()->SetAnimationSpeed(0.0f);
			}

			if (auto* battleMachine = owner_->As<BattleCharacterStateMachine>()) {
				battleMachine->CheckAndConsumeKnockBack();
			}
			timer_ = 0.0f;
		}


		void KnockBackCharacterState::Update()
		{
			// ヒットストップ中は移動・タイマー更新をスキップし、振動オフセットを更新
			if (hitStopTimer_ > 0.0f)
			{
				const float dt = g_gameTime->GetFrameDeltaTime();
				vibrationElapsed_ += dt;
				hitStopTimer_ -= dt;

				auto* characterStateMachine = owner_->As<CharacterStateMachine>();

				if (hitStopTimer_ <= 0.0f)
				{
					hitStopTimer_ = 0.0f;
					characterStateMachine->GetModelRender()->SetAnimationSpeed(1.0f);
					characterStateMachine->GetCharacter()->SetRenderPositionOffset(Vector3::Zero);
					// ヒットストップが終わってから吹き飛ばす
					if (jumpPending_)
					{
						jumpPending_ = false;
						characterStateMachine->Jump(BLOW_BACK_JUMP_POWER);
					}
				}
				else if (VIBRATION_ENABLED)
				{
					// 振幅は残り時間に比例して減衰（最初が大きく、だんだん小さく）
					const float decay = hitStopTimer_ / hitStopDuration_;
					const float wave = sinf(vibrationElapsed_ * VIBRATION_FREQUENCY);
					characterStateMachine->GetCharacter()->SetRenderPositionOffset(
						vibrationAxis_ * (wave * VIBRATION_AMPLITUDE * decay)
					);
				}
				return;
			}

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
			characterStateMachine->GetCharacter()->SetRenderPositionOffset(Vector3::Zero);
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
		{
		}


		GuardCharacterState::~GuardCharacterState()
		{
		}


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
		{
		}


		ChargeAttackCharacterState::~ChargeAttackCharacterState()
		{
		}


		void ChargeAttackCharacterState::Enter()
		{
			app::camera::CameraManager::Get().SetScreenEffectActive(true);
			chargeAttackPhase_ = ChargeAttackPhase::Start;
			chargeTimer_ = 0.0f;
			for (int i = 0; i < 3; i++)
			{
				chargeLevelEffectPlayed_[i] = false;
				chargeLevelEmitters_[i] = nullptr;
			}

			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			auto* characterStatus = characterStateMachine->GetStatus();


			// BattleCharacterStateMachineなら、チャージエフェクトの再生リクエストを出す
			if (auto* battleMachine = owner_->As<BattleCharacterStateMachine>()) {
				battleMachine->RequestChargeEffect();
				battleMachine->SetCurrentChargingLevel(0);
			}
			characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::ChargedAttackStart), 0.1f);
			app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Charging));

			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.0f);
		}


		void ChargeAttackCharacterState::Update()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();

			app::camera::CameraManager::Get().SetScreenEffectFocusWorldPos(owner_->transform.position);

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
				static constexpr float MAX_CHARGE_TIME = 2.0f;
				static constexpr float CHARGE_EFFECT_TIMES[3] = { 0.3f, 1.0f, 1.7f };
				chargeTimer_ += g_gameTime->GetFrameDeltaTime();

				// チャージレベルエフェクト（0.3s / 1.0s / 1.7s に生成 & 即再生）
				if (auto* battleMachine = owner_->As<BattleCharacterStateMachine>())
				{
					for (int i = 0; i < 3; i++)
					{
						if (!chargeLevelEffectPlayed_[i] && chargeTimer_ >= CHARGE_EFFECT_TIMES[i])
						{
							chargeLevelEmitters_[i] = battleMachine->RequestChargeLevelEffect(i);
							chargeLevelEffectPlayed_[i] = true;
							battleMachine->SetCurrentChargingLevel(i + 1);
						}
					}
				}

				// ボタン離し or チャージ上限時間で振り下ろし
				if (!characterStateMachine->IsPressA() || chargeTimer_ >= MAX_CHARGE_TIME) {
					characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::ChargedAttackEnd), 0.1f);
					app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::AtkCharge));
					chargeAttackPhase_ = ChargeAttackPhase::End;
					// 剣を振り下ろしたら被写界深度とモーションブラーを解除
					app::camera::CameraManager::Get().SetScreenEffectActive(false);
					// BattleCharacterStateMachineなら、チャージエフェクトの再生リクエストを出す
					if (auto* battleMachine = owner_->As<BattleCharacterStateMachine>()) {
						battleMachine->RequestChargeAttackEffect();
						characterStateMachine->GetModelRender()->SetAnimationSpeed(2.0f);

						// チャージレベルを確定（到達した最高レベルをセット）
						int level = 0;
						if (chargeLevelEffectPlayed_[2])      level = 3;
						else if (chargeLevelEffectPlayed_[1]) level = 2;
						else if (chargeLevelEffectPlayed_[0]) level = 1;
						battleMachine->SetChargeLevel(level);
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

							// 常にキャラクターの向きを使用
							Vector3 forward = Vector3::Front;
							csm->transform.rotation.Apply(forward);

							Vector3 attackPos =
								csm->transform.position
								+ forward * (radius + radius)
								+ Vector3(0.0f, radius, 0.0f);
							attackBody_->SetPosition(attackPos);

							/** 剣痕デカール: 壁のみ */
							if (app::effect::SwordDecalManager::IsAvailable())
							{
								Vector3 wallRayStart = csm->transform.position + Vector3(0.0f, radius, 0.0f);
								Vector3 wallRayEnd   = attackPos + forward * (radius * 1.0f);
								RaycastHit hit{};
								auto wallOnly = [](const btCollisionObject& obj) {
									return obj.getUserIndex() != enCollisionAttr_Character;
								};
								if (PhysicsWorld::Get().Raycast(wallRayStart, wallRayEnd, hit, ALL_COLLISION_ATTRIBUTE_MASK, wallOnly))
									if (fabsf(hit.normal.y) < 0.7f)
									{
										int lv = chargeLevelEffectPlayed_[2] ? 3
										       : chargeLevelEffectPlayed_[1] ? 2
										       : 1;
										app::effect::SwordDecalManager::Get().SpawnChargeDecal(
											hit.point, hit.normal, forward, lv);
									}
							}
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
			attackScheduler_.reset(nullptr);
			// 状態離脱時に被写界深度とモーションブラーを確実に解除（中断ケースのフォールバック）
			app::camera::CameraManager::Get().SetScreenEffectActive(false);

			if (auto* battleMachine = owner_->As<BattleCharacterStateMachine>())
			{
				battleMachine->SetChargeLevel(0);

				// 溜め攻撃が正常完了(End フェーズ)したときだけ通知
				if (chargeAttackPhase_ == ChargeAttackPhase::End &&
				    app::battle::BattleManager::IsAvailable())
				{
					app::battle::BattleManager::Get().NotifyChargeAttackCompleted();
				}
				battleMachine->SetCurrentChargingLevel(0);
			}

			// チャージレベルエフェクトを停止（Play済みのものだけ Stop する）
			for (int i = 0; i < 3; i++)
			{
				if (chargeLevelEmitters_[i] != nullptr)
				{
					if (chargeLevelEmitters_[i]->IsPlay())
					{
						chargeLevelEmitters_[i]->Stop();
					}
					chargeLevelEmitters_[i] = nullptr;
				}
			}

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
		{
		}


		AvoidanceCharacterState::~AvoidanceCharacterState()
		{
		}


		void AvoidanceCharacterState::Enter()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.5f);

			avoidanceDirection_ = characterStateMachine->GetMoveDirection(); // 先に確定
			timer_ = 0.0f;

			// 被ダメージ後の回避はジャスト回避を無効にする
			auto* battleSM = characterStateMachine->As<BattleCharacterStateMachine>();
			const bool allowJustDodge = (battleSM == nullptr) || !battleSM->ConsumeWasRecentlyKnockedBack();

			// [中フェーズ] 回避開始後にこのウィンドウ内で攻撃が当たるとジャスト回避になる
			justDodgeWindowTimer_ = allowJustDodge ? 0.7f : 0.0f;
			characterStateMachine->SetAvoiding(true);
			characterStateMachine->SetJustDodgeWindow(allowJustDodge);
			characterStateMachine->GetCharacterController()->SetIgnoreCharacters(true);

			characterStateMachine->OnEnterAvoidance();
		}


		void AvoidanceCharacterState::Update()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			const float realDt  = g_gameTime->GetFrameDeltaTime();
			const float localDt = characterStateMachine->GetLocalDeltaTime();

			// 回避ステート自体はローカル時間で進める（スロー中は移動が伸びる）
			timer_ += localDt;

			// ジャスト回避ウィンドウはリアルタイムで閉じる
			if (justDodgeWindowTimer_ > 0.0f)
			{
				justDodgeWindowTimer_ -= realDt;
				if (justDodgeWindowTimer_ <= 0.0f)
				{
					characterStateMachine->SetJustDodgeWindow(false);
				}
			}

			// アニメーション速度をローカルスケールに合わせる（スロー中は遅く再生）
			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.5f * characterStateMachine->GetLocalTimeScale());

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
				characterStateMachine->Move(localDt, currentSpeed);

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
			characterStateMachine->SetJustDodgeWindow(false);
			characterStateMachine->SetLocalTimeScale(1.0f); // スロー中に回避が終わった場合のリセット
			characterStateMachine->GetCharacterController()->SetIgnoreCharacters(false);
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
		{
		}


		InjuredRunCharacterState::~InjuredRunCharacterState()
		{
		}


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
			characterStateMachine->Move(g_gameTime->GetFrameDeltaTime(), characterStatus->GetMoveSpeed() * 0.5f);

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
		{
		}

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
			if (speedVec.Length() > 0.01f)
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



		/*************************************/

		WaitingAttackCharacterState::WaitingAttackCharacterState(IStateMachine* owner)
			: ICharacterState(owner)
		{
			const auto seed = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(this)) ^ static_cast<unsigned int>(time(nullptr));
			randomEngine_.seed(seed);
		}


		WaitingAttackCharacterState::~WaitingAttackCharacterState()
		{
		}


		void WaitingAttackCharacterState::Enter()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();

			characterStateMachine->GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::Idle), 0.2f);
			characterStateMachine->GetModelRender()->SetAnimationSpeed(1.0f);

			// カメラの影響を受けないようにする
			characterStateMachine->SetInputPower(1.0f);

			randomTimer_ = 0.0f;
			waitingMove_ = WaitingMove::en_wait;
			isLimitOut_ = false;
			limitOutMoveLine_ = 0.0f;
			backTime_ = 0.0f;
			limitOutForward_ = true;
		}


		void WaitingAttackCharacterState::Update()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();

			const Vector3 iPos = characterStateMachine->transform.position;
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			const Vector3 forward = characterStateMachine->GetMoveDirection();

			// ランダム移動方向の抽選
			randomTimer_ -= deltaTime;
			if (randomTimer_ <= 0.0f)
			{
				std::uniform_real_distribution<float> timeDist(0.5f, 1.5f);
				randomTimer_ = timeDist(randomEngine_);

				std::uniform_int_distribution<int> moveDist(
					static_cast<int>(WaitingMove::en_wait),
					static_cast<int>(WaitingMove::en_rightMove));
				waitingMove_ = moveDist(randomEngine_);
			}

			Vector3 right = Vector3::Right;
			right.Normalize();
			const Vector3 left = right * -1.0f;

			Vector3 moveVec = Vector3::Zero;
			switch (static_cast<WaitingMove>(waitingMove_))
			{
			case WaitingMove::en_forwardMove:
				moveVec += forward * 0.6f;
				break;
			case WaitingMove::en_backMove:
				moveVec -= forward * -0.8f;
				break;
			case WaitingMove::en_rightMove:
				moveVec += right;
				break;
			case WaitingMove::en_leftMove:
				moveVec += left;
				break;
			case WaitingMove::en_wait:
			default:
				moveVec = Vector3::Zero;
				break;
			}

			if (moveVec.LengthSq() > 0.001f)
			{
				moveVec.Normalize();
			}

			characterStateMachine->SetMoveDirection(moveVec);
			characterStateMachine->Move(deltaTime, characterStateMachine->GetStatus()->GetMoveSpeed());

			//移動している場合のみ向きを更新
			const Vector3 speedVec = characterStateMachine->GetMoveSpeedVector();
			if (speedVec.LengthSq() > 0.01f)
			{
				characterStateMachine->transform.rotation.SetRotationYFromDirectionXZ(speedVec);
			}
		}


		void WaitingAttackCharacterState::Exit()
		{
			auto* characterStateMachine = owner_->As<CharacterStateMachine>();
			characterStateMachine->SetMoveDirection(Vector3::Zero);
		}

		bool WaitingAttackCharacterState::CanChangeState() const
		{
			// 遷移条件は呼び出し元のStateMachine::Update()側で制御する
			return true;
		}
	}
}
