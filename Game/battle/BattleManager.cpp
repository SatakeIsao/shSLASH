/**
 * BattleManager.cpp
 * バトル管理
 */
#include "stdafx.h"
#include "BattleManager.h"

#include "actor/BattleCharacter.h"
#include "actor/EventCharacter.h"
#include "actor/ActorState.h"
#include "actor/CharacterSteering.h"
#include "actor/ActorStatus.h"
#include "actor/Types.h"
#include "actor/Gimmick.h"
#include "gimmick/WarpSystem.h"
#include "camera/CameraManager.h"
#include "camera/CameraController.h"
#include "core/ParameterManager.h"
#include "collision/GhostBodyManager.h"
#include "collision/CollisionHitManager.h"
#include "ui/BattleSequence.h"
#include "effect/EffectManager.h"
#include "core/PauseManager.h"
#include "core/PauseManagerObject.h"
#include "sound/SoundManager.h"


namespace
{
	constexpr const char* MASTER_BATTLE_PARAM_PATH = "Assets/master/battle/MasterBattleParameter.json";
	constexpr const char* MASTER_STAGE_PARAM_PATH = "Assets/master/battle/MasterStageParameter.json";
	constexpr const char* MASTER_BATTLE_CAMERA_PARAM_PATH = "Assets/master/battle/MasterBattleCameraParameter.json";
	constexpr const char* MASTER_BATTLE_CHARACTER_PARAM_PATH = "Assets/master/battle/MasterBattleCharacterParameter.json";
	constexpr const char* MASTER_EVENT_CHARACTER_PARAM_PATH = "Assets/master/battle/MasterEventCharacterParameter.json";

	static const int MAX_HP = 8;

	// Player用
	static app::actor::CharacterInitializeParameter sPlayerInitializeParameter = app::actor::CharacterInitializeParameter([](app::actor::CharacterInitializeParameter* parameter)
		{
			parameter->modelName = "Assets/ModelData/player/player.tkm";
			parameter->animationDataList.Create(static_cast<uint8_t>(app::actor::PlayerAnimationKind::Max));

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Idle)].filename = "Assets/animData/player/playerIdle.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Idle)].loop = true;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Run)].filename = "Assets/animData/player/playerRun.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Run)].loop = true;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpAscend)].filename = "Assets/animData/player/PlayerJump_Start.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpAscend)].loop = false;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpFalling)].filename = "Assets/animData/player/PlayerJump_Loop.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpFalling)].loop = false;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpLand)].filename = "Assets/animData/player/PlayerJump_End.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpLand)].loop = false;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Punch)].filename = "Assets/animData/player/playerPunch.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Punch)].loop = false;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::KnockBack)].filename = "Assets/animData/player/playerKnockBack.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::KnockBack)].loop = false;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Dead)].filename = "Assets/animData/player/playerDead.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Dead)].loop = false;
		});
	// Enemy用
	static app::actor::CharacterInitializeParameter sEnemyInitializeParameter = app::actor::CharacterInitializeParameter([](app::actor::CharacterInitializeParameter* parameter)
		{
			parameter->modelName = "Assets/ModelData/enemy/slime/slime.tkm";
			parameter->animationDataList.Create(static_cast<uint8_t>(app::actor::SlimeAnimationKind::Max));

			parameter->animationDataList[static_cast<uint8_t>(app::actor::SlimeAnimationKind::Idle)].filename = "Assets/animData/enemy/slime/slime_Idle.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::SlimeAnimationKind::Idle)].loop = true;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::SlimeAnimationKind::Run)].filename = "Assets/animData/enemy/slime/slime_Run.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::SlimeAnimationKind::Run)].loop = true;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::SlimeAnimationKind::Attack)].filename = "Assets/animData/enemy/slime/slime_Attack.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::SlimeAnimationKind::Attack)].loop = false;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::SlimeAnimationKind::Dead)].filename = "Assets/animData/enemy/slime/slime_Dead.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::SlimeAnimationKind::Dead)].loop = false;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::SlimeAnimationKind::knockBack)].filename = "Assets/animData/enemy/slime/slime_KnockBack.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::SlimeAnimationKind::knockBack)].loop = false;

		});

}


namespace app
{
	namespace battle
	{
		BattleManager* BattleManager::instance_ = nullptr; //初期化


		BattleManager::BattleManager()
		{
			app::gimmick::WarpSystem::Initialize();
			app::collision::CollisionHitManager::Initialize();
			app::collision::GhostBodyManager::Get().RegisterCallback([](app::collision::GhostBody* a, app::collision::GhostBody* b)
				{
					// 衝突ペア登録
					app::collision::CollisionHitManager::Get().RegisterHitPair(a, b);
				});

			layout_ = std::make_unique<app::ui::Layout>();
			layout_ ->Initialize<app::ui::MenuBase>("Assets/ui/layout/BattleSequenceMenuLayout.json");
		}


		BattleManager::~BattleManager()
		{
			DeleteGO(battleCharacter_);
			DeleteGO(eventCharacter_);
			for (auto& test : testGimmickList_)
			{
				DeleteGO(test);
			}

			// パラメーター解放
			app::core::ParameterManager::Get().UnloadParameter<app::core::MasterBattleParameter>();
			app::core::ParameterManager::Get().UnloadParameter<app::core::MasterStageParameter>();
			app::core::ParameterManager::Get().UnloadParameter<app::core::MasterBattleCharacterParameter>();
			app::collision::GhostBodyManager::Get().ClearCallback();
			app::collision::CollisionHitManager::Finalize();
			app::gimmick::WarpSystem::Finalize();
		}


		void BattleManager::Start()
		{
			// パラメーター読み込み
			LoadParameter();

			// スカイキューブ
			{
				skyCube_ = NewGO<nsK2EngineLow::SkyCube>(0, "skycube");
				//明るさを設定
				skyCube_->SetLuminance(1.0f);
				skyCube_->SetScale(300.0f);
				skyCube_->SetPosition({ 1000.0f,0.0f,1000.0f });
				//スカイキューブの種類を設定
				skyCube_->SetType((nsK2EngineLow::EnSkyCubeType)enSkyCubeType_Day);
			}
			{
				characterSteering_ = std::make_unique<app::actor::CharacterSteering>();
				// マリオにしてみた
				{
					battleCharacter_ = NewGO<app::actor::BattleCharacter>(static_cast<uint8_t>(ObjectPriority::Default), "mario");
					battleCharacter_->Initialize(sPlayerInitializeParameter);
					{
						battleCharacter_->AddState<app::actor::IdleCharacterState>();
						battleCharacter_->AddState<app::actor::RunCharacterState>();
						battleCharacter_->AddState<app::actor::JumpCharacterState>();
						battleCharacter_->AddState<app::actor::FallingCharacterState>();
						battleCharacter_->AddState<app::actor::PunchCharacterState>();
						battleCharacter_->AddState<app::actor::WarpInCharacterState>();
						battleCharacter_->AddState<app::actor::WarpOutCharacterState>();
						battleCharacter_->AddState<app::actor::KnockBackCharacterState>();
						battleCharacter_->AddState<app::actor::DeadCharacterState>();
					}
					// TODO: ステージによって変えたいので、ステージクラスが作られたら委嘱する
					{
						auto parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterStageParameter>();
						// 摩擦設定
						battleCharacter_->GetStatus()->SetFriction(parameter->friction);
						// 重力設定
						battleCharacter_->GetStatus()->SetGravity(parameter->gravity);
						// ワープ設定
						battleCharacter_->GetStatus()->SetWarpData(parameter->warpStartScale, parameter->warpEndScale, parameter->warpTime);
					}
				}
				characterSteering_->Initialize(battleCharacter_, 0);

				// 敵キャラクター
				eventCharacter_ = NewGO<app::actor::EventCharacter>(static_cast<uint8_t>(ObjectPriority::Default), "nokonoko");
				eventCharacter_->Initialize(sEnemyInitializeParameter);
				{
					eventCharacter_->AddState <app::actor::IdleCharacterState>();
					eventCharacter_->AddState<app::actor::RunCharacterState>();
					eventCharacter_->AddState<app::actor::AttackCharacterState>();
					eventCharacter_->AddState<app::actor::PunchCharacterState>();
					eventCharacter_->AddState<app::actor::DeadCharacterState>();
					eventCharacter_->AddState <app::actor::KnockBackCharacterState>();
				}
				/** 敵に重力付与のテスト */
				{
					auto stageParam = app::core::ParameterManager::Get().GetParameter<app::core::MasterStageParameter>();
					eventCharacter_->GetStatus()->SetFriction(stageParam->friction);
					eventCharacter_->GetStatus()->SetGravity(stageParam->gravity);
				}

				// ギミック設置（テスト用）
				{
					const int gimmickNum = 100;
					const int gimmickRowNum = 10;
					const int gimmickColNum = 10;
					testGimmickList_.resize(gimmickNum);

					for (int i = 0; i < testGimmickList_.size(); ++i) {
						testGimmickList_[i] = NewGO<app::actor::StaticGimmick>(static_cast<uint8_t>(ObjectPriority::Default), "testGimmick");
						// 配置
						int row = i / gimmickColNum;
						int col = i % gimmickColNum;
						float x = (static_cast<float>(col) - (gimmickColNum / 2.0f)) * 100.0f;
						float z = (static_cast<float>(row) - (gimmickRowNum / 2.0f)) * 100.0f;
						testGimmickList_[i]->transform.position = Vector3(x, -50.0f, z);
						testGimmickList_[i]->transform.scale = Vector3(1.0f, 1.0f, 1.0f);
						testGimmickList_[i]->Initialize("Assets/ModelData/stage/GroundGreenBlock.tkm");
					}
				}
				// カメラ初期化
				{
					auto parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterBattleCameraParameter>();
					cameraSteering_ = std::make_unique<app::camera::CameraSteering>();

					app::camera::CameraSteering::Config initConfig;
					initConfig.distance = parameter->distance;
					initConfig.height = parameter->height;
					initConfig.rotationSpeedX = parameter->rotationX;
					initConfig.rotationSpeedY = parameter->rotationY;
					app::camera::CameraData initData;
					initData.fov = Math::DegToRad(parameter->fov);
					initData.farClip = parameter->farClip;
					cameraSteering_->SetConfig(initConfig);
					cameraSteering_->SetTargetCharacter(battleCharacter_);

					auto gameCamera = std::make_shared<app::camera::GameCamera>();
					gameCamera->SetState(initData);
					gameCameraController_ = gameCamera;
					app::camera::CameraManager::Get().Register(app::camera::GameCamera::ID(), gameCameraController_);
					app::camera::CameraManager::Get().SwitchCamera(gameCameraController_);
				}
				//エフェクトマネージャーオブジェクト
				{
					effectManagerObject_ = NewGO<EffectManagerObject>(static_cast<uint8_t>(ObjectPriority::Default));
				}
				//ポーズマネージャーオブジェクト
				{
					pauseManagerObject_ = NewGO<app::core::PauseManagerObject>(static_cast<uint8_t>(ObjectPriority::Pause));
				}
				//バトルシーケンスマネージャーオブジェクト
				{
					battleSequenceObject_ = NewGO<app::ui::BattleSequence>(static_cast<uint8_t>(ObjectPriority::Default));
					//currentDown = Test::CountDown;
				}
				//BGM再生
				{
					app::SoundManager::Get().PlayBGM(static_cast<int>(app::SoundKind::Game));
				}
			}
		}


		void BattleManager::Update()
		{
			/** 現在のメニューポーズ状態 */
			bool currentPause = app::core::PauseManager::Get().IsPause();
			/** シーケンス中か */
			bool isSequence = false;
			if (battleSequenceObject_) {
				isSequence = battleSequenceObject_->IsPlaying();
			}

			// キャラクターたちに適用するポーズ状態（手動ポーズ中、またはシーケンス中ならポーズさせる）
			bool targetPauseState = currentPause || isSequence;

			if (isPause_ != targetPauseState)
			{
				SetPause(targetPauseState);
			}

			// シーケンス中は手動ポーズ（メニュー表示）を禁止する
			app::core::PauseManager::Get().SetCanPause(!isSequence);

			//if (isPause_ != currentPause)
			//{
			//	SetPause(currentPause);
			//}
			
			if(currentPause)
			{
				return;
			}

			/** いらないので消す */
			//if (currentDown == Test::CountDown)
			//{
			//	countDownTimer_ -= g_gameTime->GetFrameDeltaTime();
			//
			//	if (countDownTimer_ <= 0.0f) {
			//		currentDown = Test::Compleate;
			//	}
			//}

			// 上記が問題なかったら、消す
			//if (app::core::PauseManager::Get().IsPauseTrigger()) {
			//	if (app::core::PauseManager::Get().IsPause()) {
			//		// ここでGameObject止める
			//		SetPause(true);
			//	} else {
			//		// ここでGameObject動かす
			//		SetPause(false);
			//	}
			//}
			//
			//
			//if (app::core::PauseManager::Get().IsPause()) {
			//	return;
			//}


			if (!isSequence)
			{
				characterSteering_->Update();

				// 衝突判定更新
				if (app::collision::GhostBodyManager::IsAvailable()) {
					app::collision::GhostBodyManager::Get().Update();
				}
				// 衝突ヒット管理更新
				app::collision::CollisionHitManager::Get().Update();

				// デバッグテスト: 追従の処理
				Vector3 playerPosition = battleCharacter_->transform.position;
				Vector3 slimePosition = eventCharacter_->transform.position;
				//XとZのベクトルを長さに変換
				Vector3 diffXZ(playerPosition.x - slimePosition.x, 0.0f, playerPosition.z - slimePosition.z);
				float diff = diffXZ.Length();

				if (diff < 200.0f) {
					//向きだけのベクトル
					Vector3 DirectionToPlayer = diffXZ;
					DirectionToPlayer.Normalize();

					Vector3 slimeForward = Vector3(0.0f, 0.0f, 1.0f);
					eventCharacter_->transform.localRotation.Apply(slimeForward);

					//スライムの前方向
					Vector3 forwardXZ(slimeForward.x, 0.0f, slimeForward.z);
					forwardXZ.Normalize();

					//向きだけのベクトルとスライムの前方向で内積
					float dot = forwardXZ.Dot(DirectionToPlayer);

					//角度のしきい値と計算
					float halfFovDegree = 60.0f;

					float halfFovRadians = halfFovDegree * (Math::PI / 180);

					//判定用のしきい値となるコサイン値
					float threshold = std::cos(halfFovRadians);

					if (dot > threshold)
					{
						// 視野角内に入った
						eventCharacter_->GetStateMachine()->OnChase(DirectionToPlayer, playerPosition);
					}
				}

				//プレイヤーの攻撃アクション
				//TODO: Player攻撃エフェクトの修正
				{
					if (battleCharacter_->GetStateMachine()->IsPunched())
					{
						effectManagerObject_->PlayEffect(
							enEffectKind_SlimeAttack,
							battleCharacter_->transform.position + (battleCharacter_->GetStateMachine()->GetMoveDirection() * 30.0f),
							Quaternion::Identity,
							Vector3::One
						);
					}
				}

				//スライムの攻撃アクション
				{
					if (eventCharacter_->GetStateMachine()->CheckAndConsumeAttackGhostCreated()
						&& battleCharacter_->GetCurrentHP() > 0)
					{
						effectManagerObject_->PlayEffect(
							enEffectKind_SlimeAttack,
							eventCharacter_->transform.position + (eventCharacter_->GetStateMachine()->GetMoveDirection() * 30.0f) + Vector3(0.0f, 30.0f, 0.0f),
							Quaternion::Identity,
							Vector3(3.0f, 3.0f, 3.0f)
						);
					}
				}

				//スライムのノックバック
				{
					if (eventCharacter_->GetStateMachine()->IsKnockBack()
						|| eventCharacter_->GetStateMachine()->IsSquashed())
					{
						if (!hasPlayedPunchEffect_)
						{
							effectManagerObject_->PlayEffect(
								enEffectKind_SlimeKnockBack,
								eventCharacter_->transform.position,
								Quaternion::Identity,
								Vector3::One
							);
							hasPlayedPunchEffect_ = true;
						}
					}
					else {
						hasPlayedPunchEffect_ = false;
					}
				}


				// 衝突後の処理
				{
					for (auto& notify : notifyList_) {

					}
					notifyList_.clear();
				}
			}

			auto gameCamera = gameCameraController_->As<app::camera::GameCamera>();
			auto cameraData = gameCamera->GetCameraData();
			cameraSteering_->Update(cameraData, g_gameTime->GetFrameDeltaTime());
			gameCamera->SetState(cameraData);

			layout_->Update();
		}


		void BattleManager::SetPause(bool isPause)
		{
			isPause_ = isPause;
			if (battleCharacter_) battleCharacter_->SetPouse(isPause_);
			if (eventCharacter_)eventCharacter_->SetPause(isPause_);
		}


		void BattleManager::LoadParameter()
		{
			// バトル共通パラメーター読み込み
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterBattleParameter>(MASTER_BATTLE_PARAM_PATH, [](const nlohmann::json& json, app::core::MasterBattleParameter& p)
				{
					p.battleTime = json["battleTime"].get<float>();
				});
			// ステージ共通パラメーター読み込み
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterStageParameter>(MASTER_STAGE_PARAM_PATH, [](const nlohmann::json& json, app::core::MasterStageParameter& p)
				{
					p.gravity = json["gravity"].get<float>();
					p.fallLimitY = json["fallLimitY"].get<float>();
					p.friction = json["friction"].get<float>();
					p.warpStartScale = json["warpStartScale"].get<float>();
					p.warpEndScale = json["warpEndScale"].get<float>();
					p.warpTime = json["warpTime"].get<float>();
				});
			// バトルカメラパラメーター読み込み
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterBattleCameraParameter>(MASTER_BATTLE_CAMERA_PARAM_PATH, [](const nlohmann::json& json, app::core::MasterBattleCameraParameter& p)
				{
					p.distance = json["distance"].get<float>();
					p.height = json["height"].get<float>();
					p.fov = json["fov"].get<float>();
					p.nearClip = json["nearClip"].get<float>();
					p.farClip = json["farClip"].get<float>();
					p.rotationX = json["rotationX"].get<float>();
					p.rotationY = json["rotationY"].get<float>();
				});
			// バトルキャラクターパラメーター読み込み
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterBattleCharacterParameter>(MASTER_BATTLE_CHARACTER_PARAM_PATH, [](const nlohmann::json& json, app::core::MasterBattleCharacterParameter& p)
				{
					p.moveSpeed = json["moveSpeed"].get<float>();
					p.jumpMoveSpeed = json["jumpMoveSpeed"].get<float>();
					p.jumpPower = json["jumpPower"].get<float>();
					p.radius = json["radius"].get<float>();
					p.height = json["height"].get<float>();
				});
			// イベントキャラクターパラメーター読み込み
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterEventCharacterParameter>(MASTER_EVENT_CHARACTER_PARAM_PATH, [](const nlohmann::json& json, app::core::MasterEventCharacterParameter& p)
				{
					p.moveSpeed = json["moveSpeed"].get<float>();
					p.jumpMoveSpeed = json["jumpMoveSpeed"].get<float>();
					p.jumpPower = json["jumpPower"].get<float>();
					p.radius = json["radius"].get<float>();
					p.height = json["height"].get<float>();
				});
		}
	}
}