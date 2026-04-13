/**
 * BattleManager.cpp
 * バトル管理
 */
#include "stdafx.h"
#include "BattleManager.h"

#include "actor/BattleCharacter.h"
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
#include "ui/InGameUI.h"
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
	constexpr const char* MASTER_STONE_EVENT_CHARACTER_PARAM_PATH = "Assets/master/battle/MasterStoneEventCharacterParameter.json";
	constexpr const char* MASTER_MUSHROOM_EVENT_CHARACTER_PARAM_PATH = "Assets/master/battle/MasterMushroomEventCharacterParameter.json";

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
			//
			//parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpAscend)].filename = "Assets/animData/player/PlayerJump_Start.tka";
			//parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpAscend)].loop = false;
			//
			//parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpFalling)].filename = "Assets/animData/player/PlayerJump_Loop.tka";
			//parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpFalling)].loop = false;
			//
			//parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpLand)].filename = "Assets/animData/player/PlayerJump_End.tka";
			//parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::JumpLand)].loop = false;
			//
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Punch)].filename = "Assets/animData/player/playerSmallAttack.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Punch)].loop = false;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::ChargedAttackStart)].filename = "Assets/animData/player/playerChargedAttack_Start.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::ChargedAttackStart)].loop = false;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::ChargedAttackLooping)].filename = "Assets/animData/player/PlayerChargedAttack_Loop.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::ChargedAttackLooping)].loop = false;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::ChargedAttackEnd)].filename = "Assets/animData/player/PlayerChargedAttack_End.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::ChargedAttackEnd)].loop = false;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::KnockBack)].filename = "Assets/animData/player/playerKnockBack.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::KnockBack)].loop = false;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Dead)].filename = "Assets/animData/player/playerDead.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Dead)].loop = false;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Guard)].filename = "Assets/animData/player/playerGuard.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Guard)].loop = true;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Avoidance)].filename = "Assets/animData/player/playerAvoidance.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Avoidance)].loop = false;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::InjuredIdle)].filename = "Assets/animData/player/playerInjuredIdle.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::InjuredIdle)].loop = true;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::InjuredRun)].filename = "Assets/animData/player/playerInjuredRun.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::InjuredRun)].loop = true;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::KipUp)].filename = "Assets/animData/player/playerKipUp.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::KipUp)].loop = false;
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
	static app::actor::CharacterInitializeParameter sStoneEnemyInitializeParameter = app::actor::CharacterInitializeParameter([](app::actor::CharacterInitializeParameter* parameter)
		{
			parameter->modelName = "Assets/ModelData/enemy/stone/StoneMonster.tkm";
			parameter->animationDataList.Create(static_cast<uint8_t>(app::actor::StoneAnimationKind::Max));
			parameter->animationDataList[static_cast<uint8_t>(app::actor::StoneAnimationKind::Idle)].filename = "Assets/animData/enemy/stone/StoneMonstorIdle.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::StoneAnimationKind::Idle)].loop = true;
			parameter->animationDataList[static_cast<uint8_t>(app::actor::StoneAnimationKind::Run)].filename = "Assets/animData/enemy/stone/StoneMonstorRun.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::StoneAnimationKind::Run)].loop = true;
			parameter->animationDataList[static_cast<uint8_t>(app::actor::StoneAnimationKind::Attack)].filename = "Assets/animData/enemy/stone/StoneMonstorAttack.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::StoneAnimationKind::Attack)].loop = false;
			parameter->animationDataList[static_cast<uint8_t>(app::actor::StoneAnimationKind::Dead)].filename = "Assets/animData/enemy/stone/StoneMonstorDeath.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::StoneAnimationKind::Dead)].loop = false;
			parameter->animationDataList[static_cast<uint8_t>(app::actor::StoneAnimationKind::KnockBack)].filename = "Assets/animData/enemy/stone/StoneMonstorDamage.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::StoneAnimationKind::KnockBack)].loop = false;
		});
	static::app::actor::CharacterInitializeParameter sMushroomEnemyInitializeParameter = app::actor::CharacterInitializeParameter([](app::actor::CharacterInitializeParameter* parameter)
		{
			parameter->modelName = "Assets/ModelData/enemy/mushroom/Mushroom.tkm";
			parameter->animationDataList.Create(static_cast<uint8_t>(app::actor::MushroomAnimationKind::Max));
			parameter->animationDataList[static_cast<uint8_t>(app::actor::MushroomAnimationKind::Idle)].filename = "Assets/animData/enemy/mushroom/Mushroom_Idle.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::MushroomAnimationKind::Idle)].loop = true;
			parameter->animationDataList[static_cast<uint8_t>(app::actor::MushroomAnimationKind::Run)].filename = "Assets/animData/enemy/mushroom/Mushroom_Move.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::MushroomAnimationKind::Run)].loop = true;
			parameter->animationDataList[static_cast<uint8_t>(app::actor::MushroomAnimationKind::Attack)].filename = "Assets/animData/enemy/mushroom/Mushroom_Attack.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::MushroomAnimationKind::Attack)].loop = false;
			parameter->animationDataList[static_cast<uint8_t>(app::actor::MushroomAnimationKind::Dead)].filename = "Assets/animData/enemy/mushroom/Mushroom_Death.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::MushroomAnimationKind::Dead)].loop = false;
			parameter->animationDataList[static_cast<uint8_t>(app::actor::MushroomAnimationKind::KnockBack)].filename = "Assets/animData/enemy/mushroom/Mushroom_Damage.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::MushroomAnimationKind::KnockBack)].loop = false;
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
			layout_->Initialize<app::ui::MenuBase>("Assets/ui/layout/BattleSequenceMenuLayout.json");
		}


		BattleManager::~BattleManager()
		{
			DeleteGO(battleCharacter_);
			DeleteGO(eventCharacter_);
			DeleteGO(timerUIObject_);
			DeleteGO(hpUIObject_);
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
						battleCharacter_->AddState<app::actor::ChargeAttackCharacterState>();
						//battleCharacter_->AddState<app::actor::FallingCharacterState>();
						battleCharacter_->AddState<app::actor::PunchCharacterState>();
						battleCharacter_->AddState<app::actor::WarpInCharacterState>();
						battleCharacter_->AddState<app::actor::WarpOutCharacterState>();
						battleCharacter_->AddState<app::actor::KnockBackCharacterState>();
						battleCharacter_->AddState<app::actor::DeadCharacterState>();
						battleCharacter_->AddState<app::actor::GuardCharacterState>();
						battleCharacter_->AddState<app::actor::AvoidanceCharacterState>();
						battleCharacter_->AddState<app::actor::InjuredIdleCharacterState>();
						battleCharacter_->AddState<app::actor::InjuredRunCharacterState>();
						battleCharacter_->AddState<app::actor::KipUpCharacterState>();
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

				eventCharacterSpawnManager_ = std::make_unique<app::actor::EventCharacterSpawnManager>();


				eventCharacterSpawnManager_->SetOnSpawned([this](const app::actor::SpawnResult& result)
					{
						auto stageParam = app::core::ParameterManager::Get().GetParameter<app::core::MasterStageParameter>();


						switch (result.type)
						{
						case app::actor::EnemyType::STONE:
						{
							auto* stone = result.stoneCharacter;
							stoneEventCharacters_.push_back(stone);
							stone->Initialize(sStoneEnemyInitializeParameter);
							stone->AddState <app::actor::IdleCharacterState>();
							stone->AddState<app::actor::PatrolCharacterState>();
							stone->AddState<app::actor::RunCharacterState>();
							stone->AddState<app::actor::AttackCharacterState>();
							stone->AddState<app::actor::DeadCharacterState>();
							stone->AddState <app::actor::KnockBackCharacterState>();
							stone->GetStatus()->SetFriction(stageParam->friction);
							stone->GetStatus()->SetGravity(stageParam->gravity);
							break;
						}

						case app::actor::EnemyType::MUSHROOM:
						{
							auto* mushroom = result.mushroomCharacter;
							mushroomEventCharacters_.push_back(mushroom);
							mushroom->Initialize(sMushroomEnemyInitializeParameter);
							mushroom->AddState <app::actor::IdleCharacterState>();
							mushroom->AddState<app::actor::PatrolCharacterState>();
							mushroom->AddState<app::actor::RunCharacterState>();
							mushroom->AddState<app::actor::AttackCharacterState>();
							mushroom->AddState<app::actor::DeadCharacterState>();
							mushroom->AddState <app::actor::KnockBackCharacterState>();
							mushroom->GetStatus()->SetFriction(stageParam->friction);
							mushroom->GetStatus()->SetGravity(stageParam->gravity);
							break;
						}

						default:
							break;
						}

					});


				eventCharacterSpawnManager_->Start(battleCharacter_);

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
				//TODO: いま、ステージなので敵のパラメータに変更させたい
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

					for (int i = 0; i < testGimmickList_.size(); ++i)
					{
						testGimmickList_[i] = NewGO<app::actor::StaticGimmick>(static_cast<uint8_t>(ObjectPriority::Default), "testGimmick");
						//配置
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
					//battleSequenceObject_ = NewGO<app::ui::BattleSequence>(static_cast<uint8_t>(ObjectPriority::Default));
				}
				// タイマーUI
				{
					timerUIObject_ = NewGO<app::ui::TimerUIObject>(static_cast<uint8_t>(ObjectPriority::Default));
				}
				// HPUI
				{
					hpUIObject_ = NewGO<app::ui::HpUIObject>(static_cast<uint8_t>(ObjectPriority::Default));
				}
				//BGM再生
				{
					//app::SoundManager::Get().PlayBGM(static_cast<int>(app::SoundKind::Game));
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

			if (currentPause)
			{
				return;
			}

			if (!isSequence)
			{
				if (eventCharacterSpawnManager_) {
					eventCharacterSpawnManager_->Update();
				}

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
				Vector3 stonePosition = playerPosition;
				Vector3 mushroomPosition = playerPosition;
				//XとZのベクトルを長さに変換
				Vector3 diffXZ_Slime(playerPosition.x - slimePosition.x, 0.0f, playerPosition.z - slimePosition.z);
				float diff = diffXZ_Slime.Length();
				Vector3 diffXZ_Stone = Vector3::Zero;
				float diffStone = 999999.0f;
				diffXZ_Stone = Vector3(playerPosition.x - stonePosition.x, 0.0f, playerPosition.z - stonePosition.z);
				diffStone = diffXZ_Stone.Length();

				Vector3 diffXZ_Mushroom = Vector3::Zero;
				float diffMushroom = 999999.0f;
				diffXZ_Mushroom = Vector3(playerPosition.x - mushroomPosition.x, 0.0f, playerPosition.z - mushroomPosition.z);
				diffMushroom = diffXZ_Mushroom.Length();


				if (diff < 200.0f) {
					//向きだけのベクトル
					Vector3 DirectionToPlayer = diffXZ_Slime;
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

				/** ストーンの追従処理 */
				for (auto* stone : stoneEventCharacters_)
				{
					if (!stone) { continue; }
					Vector3 stonePosition = stone->transform.position;
					Vector3 diffXZ_Stone(playerPosition.x - stonePosition.x, 0.0f, playerPosition.z - stonePosition.z);
					float diffStone = diffXZ_Stone.Length();

					if (diffStone < 800.0f)
					{
						Vector3 DirectionToPlayer = diffXZ_Stone;
						DirectionToPlayer.Normalize();

						Vector3 stoneForward = Vector3(0.0f, 0.0f, 1.0f);
						stone->transform.localRotation.Apply(stoneForward);

						//ストーンの前方向
						Vector3 forwardXZ(stoneForward.x, 0.0f, stoneForward.z);
						forwardXZ.Normalize();

						//向きだけのベクトルとストーンの前方向で内積
						float dot = forwardXZ.Dot(DirectionToPlayer);

						//角度のしきい値と計算
						float halfFovDegree = 60.0f;
						float halfFovRadians = halfFovDegree * (Math::PI / 180);
						//判定用のしきい値となるコサイン値
						float threshold = std::cos(halfFovRadians);

						if (dot > threshold)
						{
							stone->GetStateMachine()->OnChase(DirectionToPlayer, playerPosition);
						}
					}
				}

				/** マッシュルームの追従処理 */
				for (auto* mushroom : mushroomEventCharacters_)
				{
					if (!mushroom) { continue; }

					Vector3 mushroomPosition = mushroom->transform.position;
					Vector3 diffXZ_Mushroom(playerPosition.x - mushroomPosition.x, 0.0f, playerPosition.z - mushroomPosition.z);
					float diffMushroom = diffXZ_Mushroom.Length();

					if (diffMushroom < 200.0f)
					{
						Vector3 DirectionToPlayer = diffXZ_Mushroom;
						DirectionToPlayer.Normalize();

						Vector3 mushroomForward = Vector3(0.0f, 0.0f, 1.0f);
						mushroom->transform.localRotation.Apply(mushroomForward);

						//マッシュルームの前方向
						Vector3 forwardXZ(mushroomForward.x, 0.0f, mushroomForward.z);
						forwardXZ.Normalize();

						//向きだけのベクトルとマッシュルームの前方向で内積
						float dot = forwardXZ.Dot(DirectionToPlayer);

						//角度のしきい値と計算
						float halfFovDegree = 60.0f;
						float halfFovRadians = halfFovDegree * (Math::PI / 180);

						//判定用のしきい値となるコサイン値
						float threshold = std::cos(halfFovRadians);

						if (dot > threshold)
						{
							mushroom->GetStateMachine()->OnChase(DirectionToPlayer, playerPosition);
						}
					}


					//プレイヤーの攻撃アクション
					{
						if (battleCharacter_->GetStateMachine()->IsPunched()
							&& !isWaitEffectPlay_)
						{
							Vector3 effectPos = battleCharacter_->transform.position + (battleCharacter_->GetStateMachine()->GetMoveDirection() * 50.0f);
							effectPos.y += 30.0f;

							Vector3 dir = battleCharacter_->GetStateMachine()->GetMoveDirection();
							reservedEffectRot_ = Quaternion::Identity;

							// 直接再生せず、予約する
							isWaitEffectPlay_ = true;
							effectDelayTimer_ = 0.5f;
							reservedEffectPos_ = effectPos;
						}

						// エフェクト再生待ち状態ならタイマーを更新
						if (isWaitEffectPlay_)
						{
							float deltaTime = g_gameTime->GetFrameDeltaTime();

							effectDelayTimer_ -= deltaTime;

							if (effectDelayTimer_ <= 0.0f)
							{
								effectManagerObject_->PlayEffect(
									enEffectKind_PlayerAttack,
									reservedEffectPos_,
									reservedEffectRot_,
									Vector3::One
								);

								isWaitEffectPlay_ = false;
							}
						}

						// チャージエフェクトの再生判定
						if (battleCharacter_->GetStateMachine()->CheckAndConsumeChargeEffectRequest())
						{
							Vector3 effectPos = battleCharacter_->transform.position + (battleCharacter_->GetStateMachine()->GetMoveDirection() * 30.0f);
							effectPos.y += 30.0f;

							effectManagerObject_->PlayEffect(
								enEffectKind_PlayerAttackCharge_Start,
								effectPos,
								Quaternion::Identity,
								Vector3::One
							);
						}

						// チャージエフェクトの再生判定
						if (battleCharacter_->GetStateMachine()->CheckAndConsumeChargeAttackEffectRequest())
						{
							Vector3 effectPos = battleCharacter_->transform.position + (battleCharacter_->GetStateMachine()->GetMoveDirection() * 30.0f);
							effectPos.y += 30.0f;

							effectManagerObject_->PlayEffect(
								enEffectKind_PlayerAttackCharge_End,
								effectPos,
								Quaternion::Identity,
								Vector3::One
							);
						}

						// ノックバックエフェクトの再生判定
						if (battleCharacter_->GetStateMachine()->GetKnockBack())
						{
							Vector3 effectPos = battleCharacter_->transform.position + (battleCharacter_->GetStateMachine()->GetMoveDirection() * 30.0f);
							effectPos.y += 30.0f;

							effectManagerObject_->PlayEffect(
								enEffectKind_SlimeAttack,
								effectPos,
								Quaternion::Identity,
								Vector3::One
							);
						}

						// 防御エフェクトの再生判定
						if (battleCharacter_->GetStateMachine()->OnGuaed())
						{
							Vector3 effectPos = battleCharacter_->transform.position + (battleCharacter_->GetStateMachine()->GetMoveDirection() * 30.0f);
							effectPos.y += 30.0f;

							effectManagerObject_->PlayEffect(
								enEffectKind_SlimeAttack,
								effectPos,
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

				/** 制限時間の管理 */
				{
					if (!timerUIObject_) return;

					if (remainTime_ > 0.0f)
					{
						remainTime_ -= g_gameTime->GetFrameDeltaTime();
					}

					if (timerUIObject_) {
						timerUIObject_->SetTimer(remainTime_);
					}
				}

				layout_->Update();
			}
		}


			void BattleManager::SetPause(bool isPause)
			{
				isPause_ = isPause;
				if (battleCharacter_) battleCharacter_->SetPouse(isPause_);
				if (eventCharacter_)eventCharacter_->SetPause(isPause_);
				for (auto* stone : stoneEventCharacters_)
				{
					if (stone) { stone->SetPause(isPause_); }
				}

				for (auto* mushroom : mushroomEventCharacters_)
				{
					if (mushroom) { mushroom->SetPause(isPause_); }
				}
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
				app::core::ParameterManager::Get().LoadParameter<app::core::MasterStoneEventCharacterParameter>(MASTER_STONE_EVENT_CHARACTER_PARAM_PATH, [](const nlohmann::json& json, app::core::MasterStoneEventCharacterParameter& p)
					{
						p.moveSpeed = json["moveSpeed"].get<float>();
						p.jumpMoveSpeed = json["jumpMoveSpeed"].get<float>();
						p.jumpPower = json["jumpPower"].get<float>();
						p.radius = json["radius"].get<float>();
						p.height = json["height"].get<float>();
					});
				app::core::ParameterManager::Get().LoadParameter<app::core::MasterMushroomEventCharacterParameter>(MASTER_MUSHROOM_EVENT_CHARACTER_PARAM_PATH, [](const nlohmann::json& json, app::core::MasterMushroomEventCharacterParameter& p)
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