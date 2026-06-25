/**
 * BattleManager.cpp
 * バトル管理
 */
#include "stdafx.h"
#include "BattleManager.h"
#include "effect/SwordDecalManager.h"

#include "actor/BattleCharacter.h"
#include "actor/EventCharacterSpawnManagerObject.h"
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
#include "ui/DamagePopUI.h"
#include "ui/InGameUI.h"
#include "effect/EffectManager.h"
#include "core/PauseManager.h"
#include "core/PauseManagerObject.h"
#include "EnemyPhase.h"
#include "sound/SoundManager.h"
#include "GameResultData.h"


namespace
{
	/** 敵スポーンのON/OFF */
	constexpr bool ENABLE_ENEMY_SPAWN = true;

	constexpr const char* MASTER_BATTLE_PARAM_PATH = "Assets/master/battle/MasterBattleParameter.json";
	constexpr const char* MASTER_STAGE_PARAM_PATH = "Assets/master/battle/MasterStageParameter.json";
	constexpr const char* MASTER_BATTLE_CAMERA_PARAM_PATH = "Assets/master/battle/MasterBattleCameraParameter.json";
	constexpr const char* MASTER_BATTLE_CHARACTER_PARAM_PATH = "Assets/master/battle/MasterBattleCharacterParameter.json";
	constexpr const char* MASTER_WEAPON_PARAM_PATH = "Assets/master/battle/MasterWeaponParameter.json";
	constexpr const char* MASTER_EVENT_CHARACTER_PARAM_PATH = "Assets/master/battle/MasterEventCharacterParameter.json";
	constexpr const char* MASTER_STONE_EVENT_CHARACTER_PARAM_PATH = "Assets/master/battle/MasterStoneEventCharacterParameter.json";
	constexpr const char* MASTER_MUSHROOM_EVENT_CHARACTER_PARAM_PATH = "Assets/master/battle/MasterMushroomEventCharacterParameter.json";

	/** 無敵時間 */
	static constexpr float INVINCIBLE_TIME = 2.0f;
	/** エフェクトのY座標調整 */
	static constexpr float STONE_SPAWN_EFFECT_OFFSET_Y = 180.0f;
	static constexpr float MUSHROOM_SPAWN_EFFECT_OFFSET_Y = 180.0f;

	/** 溜め攻撃、命中時の視野角 */
	struct ChargeHitFovPreset
	{
		float fov;
	};

	/** 溜め攻撃時用のFOVプリセット */
	constexpr ChargeHitFovPreset CHARGE_HIT_FOV_PRESETS[] = {
		{ 55.0f },
		{ 55.0f },
		{ 55.0f },
	};
	/** ヒット時FOV縮小時間（ズームイン） */
	constexpr float CHARGE_HIT_FOV_FADE_IN = 0.15f;
	/** ヒット時FOV復帰時間（ズームアウト） */
	constexpr float CHARGE_HIT_FOV_FADE_OUT = 0.12f;
	/** 待機状態判定のしきい値 */
	constexpr float CHARGE_HIT_FOV_IDLE_THRESHOLD = 0.01f;
	/** プリセットの最大インデックス */
	constexpr int CHARGE_HIT_FOV_MAX_INDEX = 2;

	// Player
	static app::actor::CharacterInitializeParameter sPlayerInitializeParameter = app::actor::CharacterInitializeParameter([](app::actor::CharacterInitializeParameter* parameter)
		{
			parameter->modelName = "Assets/ModelData/player/player.tkm";
			parameter->animationDataList.Create(static_cast<uint8_t>(app::actor::PlayerAnimationKind::Max));

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Idle)].filename = "Assets/animData/player/playerIdle.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Idle)].loop = true;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Run)].filename = "Assets/animData/player/playerRun.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::Run)].loop = true;
		
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::SlashFirst)].filename = "Assets/animData/player/playerSmallAttack_First.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::SlashFirst)].loop = false;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::SlashSecond)].filename = "Assets/animData/player/playerSmallAttack_Second.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::SlashSecond)].loop = false;

			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::SlashThird)].filename = "Assets/animData/player/playerSmallAttack_Third.tka";
			parameter->animationDataList[static_cast<uint8_t>(app::actor::PlayerAnimationKind::SlashThird)].loop = false;

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
			app::effect::SwordDecalManager::Initialize();
			app::gimmick::WarpSystem::Initialize();
			app::collision::CollisionHitManager::Initialize();
			app::collision::GhostBodyManager::Get().RegisterCallback([](app::collision::GhostBody* a, app::collision::GhostBody* b)
				{
					// 衝突ペア登録
					app::collision::CollisionHitManager::Get().RegisterHitPair(a, b);
				});
		}


		BattleManager::~BattleManager()
		{
			// リザルト用データを保存（playerHpUIObject_ 削除前に取得する）
			{
				auto& result = app::GameResultData::Get();
				result.Reset();
				result.level = playerHpUIObject_ ? playerHpUIObject_->GetLevel() : 0;
				result.stoneKillCount = stoneKillCount_;
				result.mushroomKillCount = mushroomKillCount_;
			}

			// スポーン済み敵とHPバーを先にクリーンアップ
			if (eventCharacterSpawnManagerObject_)
			{
				eventCharacterSpawnManagerObject_->GetManager().CleanUp();
			}

			DeleteGO(skyCube_);
			DeleteGO(moon_);
			DeleteGO(battleCharacter_);
			DeleteGO(battleSequenceObject_);
			DeleteGO(effectManagerObject_);
			DeleteGO(effectManager2DObject_);
			DeleteGO(pauseManagerObject_);
			DeleteGO(eventCharacterSpawnManagerObject_);
			DeleteGO(eventCharacter_);
			DeleteGO(timerUIObject_);
			DeleteGO(playerHpUIObject_);
			DeleteGO(enemyHpUIObject_);
			DeleteGO(levelUpObject_);
			DeleteGO(phaseUI_);
			if (damagePopPool_)
			{
				damagePopPool_->Finalize();
				delete damagePopPool_;
				damagePopPool_ = nullptr;
			}
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
			app::effect::SwordDecalManager::Finalize();
		}


		void BattleManager::Start()
		{
			// DebugScene 用の同期版。BattleScene/TutorialScene は LoadStep() を使う。
			while (!LoadStep()) {}
		}


		bool BattleManager::LoadStep()
		{
			switch (loadStep_++)
			{
			case 0:
				// パラメーター読み込み
				LoadParameter();
				return false;

			case 1:
				// スカイキューブ + 月オブジェクト
				{
					skyCube_ = NewGO<nsK2EngineLow::SkyCube>(0, "skycube");
					skyCube_->SetLuminance(1.0f);
					skyCube_->SetScale(300.0f);
					skyCube_->SetPosition({ 1000.0f, 0.0f, 1000.0f });
					skyCube_->SetType((nsK2EngineLow::EnSkyCubeType)enSkyCubeType_NightToon_2);
				}
				{
					moon_ = NewGO<app::actor::MoonGimmick>(static_cast<uint8_t>(ObjectPriority::Default), "moon");
					moon_->transform.position = Vector3(1000.0f, 1800.0f, 5000.0f);
					moon_->transform.scale = Vector3(26.0f, 26.0f, 26.0f);
					moon_->Initialize("Assets/ModelData/stage/moon.tkm");
				}
				return false;

			case 2:
				// エフェクトマネージャー + スポーンマネージャー（コールバックはラムダなので遅延実行）
				{
					effectManagerObject_ = NewGO<EffectManagerObject>(static_cast<uint8_t>(ObjectPriority::Default));
					effectManager2DObject_ = NewGO<EffectManager2DObject>(static_cast<uint8_t>(ObjectPriority::Default));
				}
				{
					eventCharacterSpawnManagerObject_ = NewGO<app::actor::EventCharacterSpawnManagerObject>(static_cast<uint8_t>(ObjectPriority::Default));
					eventCharacterSpawnManagerObject_->GetManager().SetOnSpawned([this](const app::actor::SpawnResult& result)
						{
							auto stageParam = app::core::ParameterManager::Get().GetParameter<app::core::MasterStageParameter>();
							switch (result.type)
							{
							case app::actor::EnemyType::STONE:
							{
								auto* stone = result.stoneCharacter;

								stoneEventCharacters_.push_back(stone);
								if (!stone->GetModelRender())
								{
									stone->Initialize(sStoneEnemyInitializeParameter);
									stone->AddState <app::actor::IdleCharacterState>();
									stone->AddState<app::actor::PatrolCharacterState>();
									stone->AddState<app::actor::RunCharacterState>();
									stone->AddState<app::actor::AttackCharacterState>();
									stone->AddState<app::actor::WaitingAttackCharacterState>();
									stone->AddState<app::actor::DeadCharacterState>();
									stone->AddState <app::actor::KnockBackCharacterState>();
								}
								stone->GetStatus()->SetFriction(stageParam->friction);
								stone->GetStatus()->SetGravity(stageParam->gravity);
								stone->GetCharacterController()->SetGravity(stageParam->gravity);
								if (isTutorialMode_)
									stone->GetStateMachine()->SetAIEnabled(false);
								if (effectManagerObject_)
								{
									static Vector3 spawnEffectPos = result.spawnPosition;
									spawnEffectPos = result.spawnPosition;
									spawnEffectPos.y += STONE_SPAWN_EFFECT_OFFSET_Y;
									effectManagerObject_->PlayEffectFollow(
										enEffectKind_StoneSpawn,
										&spawnEffectPos,
										Quaternion::Identity,
										Vector3::One
									);
									{
										const auto& sp = *app::core::ParameterManager::Get().GetParameter<app::core::MasterStoneEventCharacterParameter>();
										TriggerTBDRSpawnLight(spawnEffectPos, Vector3(sp.spawnLightColorR, sp.spawnLightColorG, sp.spawnLightColorB), 250.f, 2.5f);
									}
								}
								stone->AddOnDeadEffect([this, stone]()
								{
									if (effectManagerObject_)
									{
										effectManagerObject_->PlayEffect(
											enEffectKind_StoneDead,
											stone->transform.position,
											Quaternion::Identity,
											Vector3::One
										);
									}
									if (app::effect::SwordDecalManager::IsAvailable())
									{
										Vector3 rayStart = stone->transform.position + Vector3(0.0f, 50.0f, 0.0f);
										Vector3 rayEnd   = stone->transform.position + Vector3(0.0f, -200.0f, 0.0f);
										RaycastHit hit{};
										if (PhysicsWorld::Get().Raycast(rayStart, rayEnd, hit))
											if (hit.normal.y > 0.7f)
												app::effect::SwordDecalManager::Get().SpawnDecal(
													hit.point, hit.normal, Vector3::Front);
									}
								});
								stone->AddOnDead([this, stone]()
								{
									stoneEventCharacters_.erase(
										std::remove(stoneEventCharacters_.begin(), stoneEventCharacters_.end(), stone),
										stoneEventCharacters_.end()
									);
									if (playerHpUIObject_)
									{
										playerHpUIObject_->AddLevelUpGauge(3);
										app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::GaugeUp));
									}
									stoneKillCount_++;
								});
								break;
							}
							case app::actor::EnemyType::MUSHROOM:
							{
								auto* mushroom = result.mushroomCharacter;

								mushroomEventCharacters_.push_back(mushroom);
								if (!mushroom->GetModelRender())
								{
									mushroom->Initialize(sMushroomEnemyInitializeParameter);
									mushroom->AddState <app::actor::IdleCharacterState>();
									mushroom->AddState<app::actor::PatrolCharacterState>();
									mushroom->AddState<app::actor::RunCharacterState>();
									mushroom->AddState<app::actor::AttackCharacterState>();
									mushroom->AddState<app::actor::WaitingAttackCharacterState>();
									mushroom->AddState<app::actor::DeadCharacterState>();
									mushroom->AddState <app::actor::KnockBackCharacterState>();
								}
								mushroom->GetStatus()->SetFriction(stageParam->friction);
								mushroom->GetStatus()->SetGravity(stageParam->gravity);
								mushroom->GetCharacterController()->SetGravity(stageParam->gravity);
								if (isTutorialMode_)
									mushroom->GetStateMachine()->SetAIEnabled(false);
								if (effectManagerObject_)
								{
									static Vector3 spawnEffectPos = result.spawnPosition;
									spawnEffectPos = result.spawnPosition;
									spawnEffectPos.y += MUSHROOM_SPAWN_EFFECT_OFFSET_Y;
									effectManagerObject_->PlayEffectFollow(
										enEffectKind_MushroomSpawn,
										&spawnEffectPos,
										Quaternion::Identity,
										Vector3(1.2f, 1.2f, 1.2f)
									);
									{
										const auto& mp = *app::core::ParameterManager::Get().GetParameter<app::core::MasterMushroomEventCharacterParameter>();
										TriggerTBDRSpawnLight(spawnEffectPos, Vector3(mp.spawnLightColorR, mp.spawnLightColorG, mp.spawnLightColorB), 250.f, 2.5f);
									}
								}
								mushroom->AddOnDeadEffect([this, mushroom]()
								{
									if (effectManagerObject_)
									{
										effectManagerObject_->PlayEffect(
											enEffectKind_MushroomDead,
											mushroom->transform.position,
											Quaternion::Identity,
											Vector3::One
										);
									}
									if (app::effect::SwordDecalManager::IsAvailable())
									{
										Vector3 rayStart = mushroom->transform.position + Vector3(0.0f, 50.0f, 0.0f);
										Vector3 rayEnd   = mushroom->transform.position + Vector3(0.0f, -200.0f, 0.0f);
										RaycastHit hit{};
										if (PhysicsWorld::Get().Raycast(rayStart, rayEnd, hit))
											if (hit.normal.y > 0.7f)
												app::effect::SwordDecalManager::Get().SpawnMushroomFloorDecal(
													hit.point, hit.normal, Vector3::Front);
									}
								});
								mushroom->AddOnDead([this, mushroom]()
								{
									mushroomEventCharacters_.erase(
										std::remove(mushroomEventCharacters_.begin(), mushroomEventCharacters_.end(), mushroom),
										mushroomEventCharacters_.end()
									);
									if (playerHpUIObject_)
									{
										playerHpUIObject_->AddLevelUpGauge(5);
										app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::GaugeUp));
									}
									mushroomKillCount_++;
								});
								break;
							}
							default:
								break;
							}
						});
				}
				return false;

			case 3:
				// プレイヤー + キャラクターステアリング + スポーンマネージャー起動
				{
					characterSteering_ = std::make_unique<app::actor::CharacterSteering>();
					{
						battleCharacter_ = NewGO<app::actor::BattleCharacter>(static_cast<uint8_t>(ObjectPriority::Character), "mario");
						battleCharacter_->Initialize(sPlayerInitializeParameter);
						{
							battleCharacter_->AddState<app::actor::IdleCharacterState>();
							battleCharacter_->AddState<app::actor::RunCharacterState>();
							battleCharacter_->AddState<app::actor::ChargeAttackCharacterState>();
							battleCharacter_->AddState<app::actor::FallingCharacterState>();
							battleCharacter_->AddState<app::actor::SlashFirstCharacterState>();
							battleCharacter_->AddState<app::actor::SlashSecondCharacterState>();
							battleCharacter_->AddState<app::actor::SlashThirdCharacterState>();
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
						{
							PendingSpawnEffect entry;
							entry.effectKind = enEffectKind_PlayerLevelUp;
							entry.scale = Vector3::One;
							entry.timer = 0.1f;
							pendingSpawnEffects_.push_back(entry);
							pendingPlayerSpawnLightTimer_ = 0.1f;
						}
						{
							auto parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterStageParameter>();
							battleCharacter_->GetStatus()->SetFriction(parameter->friction);
							battleCharacter_->GetStatus()->SetGravity(parameter->gravity);
							battleCharacter_->GetStatus()->SetWarpData(parameter->warpStartScale, parameter->warpEndScale, parameter->warpTime);
						}
					}
					battleCharacter_->GetStateMachine()->SetJustDodgeCallback([this]()
					{
						AddPlayerGauge(2);
					});
					characterSteering_->Initialize(battleCharacter_, 0);
					eventCharacterSpawnManagerObject_->GetManager().SetFieldEdge(300.0f);
					eventCharacterSpawnManagerObject_->GetManager().SetSpawnPosY(-354.0f);
					if (isTutorialMode_)
					{
						eventCharacterSpawnManagerObject_->GetManager().Start(battleCharacter_);
						eventCharacterSpawnManagerObject_->GetManager().ResetPendingSpawn();
						eventCharacterSpawnManagerObject_->GetManager().SetTutorialMode(true);
						tutorialNeedsSpawn_ = true;
					}
					else if (ENABLE_ENEMY_SPAWN)
					{
						eventCharacterSpawnManagerObject_->GetManager().Start(battleCharacter_);
					}
					battleCharacter_->SetSpawnManager(&eventCharacterSpawnManagerObject_->GetManager());
				}
				return false;

			case 4:
				// ステージギミック（stage.tkm はサイズが大きいため単独ステップ）
				{
					testGimmickList_.resize(1);
					testGimmickList_[0] = NewGO<app::actor::StaticGimmick>(static_cast<uint8_t>(ObjectPriority::Default), "testGimmick");
					testGimmickList_[0]->transform.position = Vector3(0.0f, -50.0f, 0.0f);
					testGimmickList_[0]->transform.scale = Vector3(5.0f, 5.0f, 5.0f);
					testGimmickList_[0]->Initialize("Assets/ModelData/stage/stage.tkm", "Assets/Shader/modelWall.fx", "Assets/Shader/model_gbuffer_wall.fx");
				}
				return false;

			case 5:
				// カメラ + ポーズ + UI + BGM
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
				{
					pauseManagerObject_ = NewGO<app::core::PauseManagerObject>(static_cast<uint8_t>(ObjectPriority::Pause));
				}
				if (!isTutorialMode_)
				{
					timerUIObject_ = NewGO<app::ui::TimerUIObject>(static_cast<uint8_t>(ObjectPriority::Default));
					timerUIObject_->SetTimer(remainTime_);
				}
				{
					playerHpUIObject_ = NewGO<app::ui::PlayerHpUIObject>(static_cast<uint8_t>(ObjectPriority::PlayerUI));
				}
				if (playerHpUIObject_ && battleCharacter_)
				{
					playerHpUIObject_->SetPlayer(battleCharacter_);
				}
				isInvincible_ = true;
				invincibleTimer_ = 3.0f;
				{
					app::SoundManager::Get().PlayBGM(static_cast<int>(app::SoundKind::Game));
				}
				if (!isTutorialMode_)
				{
					levelUpObject_ = NewGO<app::ui::LevelUpUIObject>(static_cast<uint8_t>(ObjectPriority::PlayerUI));
					if (playerHpUIObject_ && levelUpObject_)
					{
						playerHpUIObject_->SetLevelUpUIObject(levelUpObject_);
					}
					phaseUI_ = NewGO<app::actor::PhaseUI>(static_cast<uint8_t>(ObjectPriority::Default));
					eventCharacterSpawnManagerObject_->GetManager().SetPhaseUI(phaseUI_);
				}
				{
					damagePopPool_ = new app::ui::DamagePopPool();
					damagePopPool_->Initialize();
					SetDamagePopListener(damagePopPool_);
				}
				return true;

			default:
				return true;
			}
		}


		void BattleManager::Update()
		{
			// タイムアップフリーズ更新（ポーズ・シーケンス状態に関係なく常に実行）
			if (timeUpFreezeFrames_ > 0)
			{
				--timeUpFreezeFrames_;
				if (timeUpFreezeFrames_ == 0)
				{
					g_gameTime->DisableFixedFrameDeltaTime();
					if (battleSequenceObject_)
					{
						battleSequenceObject_->PlayTimeUp();
					}
				}
			}

			// 剣痕デカールの寿命管理
			if (app::effect::SwordDecalManager::IsAvailable()) {
				app::effect::SwordDecalManager::Get().Update();
			}

			UpdateTBDRSpawnLights();

			// アニメーション初期化フレームを確保してからシーケンスを生成する
			// （生成直後に SetPause するとアニメが走らず T ポーズになるため）
			// チュートリアル：EffectManager の初期化完了後に敵を固定配置する
			if (tutorialNeedsSpawn_ && EffectManager::IsAvailable() && IsOpeningSequenceDone())
			{
				tutorialNeedsSpawn_ = false;
				tutorialEnemySpawnRot_.SetRotationY(Math::PI);
				tutorialStoneSpawnPos_    = Vector3(150.0f,  -354.0f, 0.0f);
				tutorialMushroomSpawnPos_ = Vector3(-150.0f, -354.0f, 0.0f);
				eventCharacterSpawnManagerObject_->GetManager().SpawnFixed(
					app::actor::EnemyType::STONE,    tutorialStoneSpawnPos_,    tutorialEnemySpawnRot_);
				eventCharacterSpawnManagerObject_->GetManager().SpawnFixed(
					app::actor::EnemyType::MUSHROOM, tutorialMushroomSpawnPos_, tutorialEnemySpawnRot_);
			}

			// チュートリアル練習フェーズ前：全滅時にリスポーン
			if (tutorialRespawnEnabled_ && IsOpeningSequenceDone() && !tutorialNeedsSpawn_)
			{
				if (stoneEventCharacters_.empty())
					eventCharacterSpawnManagerObject_->GetManager().SpawnFixed(
						app::actor::EnemyType::STONE, tutorialStoneSpawnPos_, tutorialEnemySpawnRot_);
				if (mushroomEventCharacters_.empty())
					eventCharacterSpawnManagerObject_->GetManager().SpawnFixed(
						app::actor::EnemyType::MUSHROOM, tutorialMushroomSpawnPos_, tutorialEnemySpawnRot_);
			}

			// カウントダウン＆スタート演出（チュートリアルでは不要）
			if (!isTutorialMode_ && !battleSequenceObject_)
			{
				battleSequenceStartTimer_ -= g_gameTime->GetFrameDeltaTime();
				if (battleSequenceStartTimer_ <= 0.0f)
				{
					battleSequenceObject_ = NewGO<app::ui::BattleSequence>(static_cast<uint8_t>(ObjectPriority::SequenceUI));
				}
			}

			/** 現在のメニューポーズ状態 */
			bool currentPause = app::core::PauseManager::Get().IsPause();
			// シーケンスが終了していない間はポーズ・入力をすべて封印
			bool isSequence = battleSequenceObject_ && !battleSequenceObject_->IsFinished();
			// キャラクターたちに適用するポーズ状態（手動ポーズ中、シーケンス中、またはチュートリアルフリーズ中ならポーズさせる）
			bool targetPauseState = currentPause || isSequence || tutorialFreeze_;

			if (isPause_ != targetPauseState)
			{
				SetPause(targetPauseState);
			}

			// シーケンス中は手動ポーズ（メニュー表示）を禁止する
			app::core::PauseManager::Get().SetCanPause(!isSequence);

			if (currentPause || tutorialFreeze_ || gameOverFreeze_)
			{
				return;
			}

			/** 遅延再生処理 */
			if (effectManagerObject_)
			{
				for (auto it = pendingSpawnEffects_.begin(); it != pendingSpawnEffects_.end(); )
				{
					it->timer -= g_gameTime->GetFrameDeltaTime();
					if (it->timer <= 0.0f)
					{
						playerSpawnEffectPos_ = battleCharacter_->transform.position;
						effectManagerObject_->PlayEffectFollow(
							it->effectKind,
							&playerSpawnEffectPos_,
							Quaternion::Identity,
							it->scale
						);
						it = pendingSpawnEffects_.erase(it);
					}
					else
					{
						++it;
					}
				}
			}

			// プレイヤースポーン時のスポットライト（シーケンス終了時に発火）
			if (pendingPlayerSpawnLightTimer_ >= 0.0f)
			{
				pendingPlayerSpawnLightTimer_ -= g_gameTime->GetFrameDeltaTime();
				if (pendingPlayerSpawnLightTimer_ <= 0.0f)
				{
					g_sceneLight->TriggerSpawnLight(battleCharacter_->transform.position);
					{
						const auto& bp = *app::core::ParameterManager::Get().GetParameter<app::core::MasterBattleCharacterParameter>();
						TriggerTBDRSpawnLight(
							battleCharacter_->transform.position + Vector3(0.f, 80.f, 0.f),
							Vector3(bp.spawnLightColorR, bp.spawnLightColorG, bp.spawnLightColorB),
							bp.spawnLightRange, bp.spawnLightDuration);
					}
					pendingPlayerSpawnLightTimer_ = -1.0f;
				}
			}

#if defined(APP_DEBUG)
			// デバッグ機能：LB1+Downでプレイヤーに1ダメージ
			if (g_pad[0]->IsPress(enButtonLB1) && g_pad[0]->IsTrigger(enButtonDown))
			{
				float newHp = max(battleCharacter_->GetStatus()->GetCurrentHp() - 1.0f, 0.0f);
				battleCharacter_->GetStatus()->SetCurrentHp(newHp);
				if (newHp <= 0.0f)
				{
					battleCharacter_->GetStateMachine()->OnDead();
				}
				else
				{
					battleCharacter_->GetStateMachine()->OnKnockBack();
				}
			}

			// デバッグ機能：LB1+UpでタイムアップUIをトグル表示
			if (g_pad[0]->IsPress(enButtonLB1) && g_pad[0]->IsTrigger(enButtonUp))
			{
				if (battleSequenceObject_ && battleSequenceObject_->IsFinished())
				{
					battleSequenceObject_->DebugToggleTimeUp();
				}
			}

			// デバッグ機能：LB1+RightでタイマーをX秒増加、LB1+Leftで減少
			if (timerUIObject_)
			{
				constexpr float kDebugTimerStep = 5.0f;
				if (g_pad[0]->IsPress(enButtonLB1) && g_pad[0]->IsTrigger(enButtonRight))
				{
					timerUIObject_->AddTimer(+kDebugTimerStep);
					lastCountShown_ = -1;  // カウントダウン表示をリセット
				}
				if (g_pad[0]->IsPress(enButtonLB1) && g_pad[0]->IsTrigger(enButtonLeft))
				{
					timerUIObject_->AddTimer(-kDebugTimerStep);
				}
			}
#endif // APP_DEBUG

			if (!isSequence)
			{
				if (playerInputEnabled_)
					characterSteering_->Update();
				else if (battleCharacter_)
					battleCharacter_->GetStateMachine()->ClearInput();

				// 衝突判定更新
				if (app::collision::GhostBodyManager::IsAvailable()) {
					app::collision::GhostBodyManager::Get().Update();
				}
				// 衝突ヒット管理更新
				app::collision::CollisionHitManager::Get().Update();

				//// デバッグテスト: 追従の処理
				Vector3 playerPosition = battleCharacter_->transform.position;
				////Vector3 slimePosition = eventCharacter_->transform.position;
				Vector3 stonePosition = playerPosition;
				Vector3 mushroomPosition = playerPosition;
				////XとZのベクトルを長さに変換
				////Vector3 diffXZ_Slime(playerPosition.x - slimePosition.x, 0.0f, playerPosition.z - slimePosition.z);
				////float diff = diffXZ_Slime.Length();
				Vector3 diffXZ_Stone = Vector3::Zero;
				float diffStone = 999999.0f;
				diffXZ_Stone = Vector3(playerPosition.x - stonePosition.x, 0.0f, playerPosition.z - stonePosition.z);
				diffStone = diffXZ_Stone.Length();
				//
				Vector3 diffXZ_Mushroom = Vector3::Zero;
				float diffMushroom = 999999.0f;
				diffXZ_Mushroom = Vector3(playerPosition.x - mushroomPosition.x, 0.0f, playerPosition.z - mushroomPosition.z);
				diffMushroom = diffXZ_Mushroom.Length();
				//
				//
				//if (diff < 200.0f) {
				//	//向きだけのベクトル
				//	Vector3 DirectionToPlayer = diffXZ_Slime;
				//	DirectionToPlayer.Normalize();
				//
				//	Vector3 slimeForward = Vector3(0.0f, 0.0f, 1.0f);
				//	eventCharacter_->transform.localRotation.Apply(slimeForward);
				//
				//	//スライムの前方向
				//	Vector3 forwardXZ(slimeForward.x, 0.0f, slimeForward.z);
				//	forwardXZ.Normalize();
				//
				//	//向きだけのベクトルとスライムの前方向で内積
				//	float dot = forwardXZ.Dot(DirectionToPlayer);
				//
				//	//角度のしきい値と計算
				//	float halfFovDegree = 60.0f;
				//
				//	float halfFovRadians = halfFovDegree * (Math::PI / 180);
				//
				//	//判定用のしきい値となるコサイン値
				//	float threshold = std::cos(halfFovRadians);
				//
				//	if (dot > threshold)
				//	{
				//		// 視野角内に入った
				//		eventCharacter_->GetStateMachine()->OnChase(DirectionToPlayer, playerPosition);
				//	}
				//}

				if (!isTutorialMode_ || tutorialEnemyMoveEnabled_)
				{
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

						if (dot > threshold || tutorialEnemyMoveEnabled_)
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

						if (dot > threshold || tutorialEnemyMoveEnabled_)
						{
							mushroom->GetStateMachine()->OnChase(DirectionToPlayer, playerPosition);
						}
					}
				}
				} // if (!isTutorialMode_ || tutorialEnemyMoveEnabled_)

				// 無敵時間の更新
				if (isInvincible_)
				{
					invincibleTimer_ -= g_gameTime->GetFrameDeltaTime();
					if (invincibleTimer_ <= 0.0f)
					{
						invincibleTimer_ = 0.0f;
						isInvincible_ = false;
						battleCharacter_->GetStateMachine()->SetInvincible(false);
					}
				}

				if (guardSuccessCooldown_ > 0.0f)
				{
					guardSuccessCooldown_ -= g_gameTime->GetFrameDeltaTime();
				}

				//プレイヤーの攻撃アクション
				{
					if (battleCharacter_->GetStateMachine()->IsSlashEffect()
						&& !isWaitEffectPlay_)
					{
						// プレイヤーのモデルが向いている方向を前方ベクトルとして取得
						Vector3 forward = Vector3(0.0f, 0.0f, 1.0f);
						battleCharacter_->transform.localRotation.Apply(forward);
						forward.y = 0.0f;
						forward.Normalize();

						// 攻撃開始時の向きを固定
						reservedEffectDir_ = forward;

						// 回転を固定
						if (reservedEffectDir_.LengthSq() > 0.0001f)
						{
							float yaw = atan2f(reservedEffectDir_.x, reservedEffectDir_.z) + Math::PI / 2.0f;
							reservedEffectRot_.SetRotationY(yaw);

							// 斬撃モーションに合わせた傾きを追加（角度は要調整）
							Quaternion tilt;
							tilt.SetRotationX(Math::DegToRad(-45.0f));  // 45度は要調整
							reservedEffectRot_ = reservedEffectRot_ * tilt;
						}
						else
						{
							reservedEffectRot_ = Quaternion::Identity;
						}

						bool isFirstSlash = battleCharacter_->GetStateMachine()->IsSlashFirst();
						bool isSecondSlash = battleCharacter_->GetStateMachine()->isSlashSecond();
						isWaitEffectPlay_ = true;
						effectDelayTimer_ = isFirstSlash ? 0.0f : 0.3f;

						/** 2回目だけ回転を変更 */
						if (isSecondSlash)
						{
							Quaternion flipX;
							flipX.SetRotationX(Math::PI);
							reservedEffectRot_ = reservedEffectRot_ * flipX;
						}
						 
						battleCharacter_->GetStateMachine()->SetSlashEffect(false);
					}

					// エフェクト再生待ち状態ならタイマーを更新
					if (isWaitEffectPlay_)
					{
						// 座標はプレイヤーに追従、向きは reservedEffectDir_（固定）でオフセット
						reservedEffectPos_ = battleCharacter_->transform.position
							+ (reservedEffectDir_ * 30.0f);
						reservedEffectPos_.y += 30.0f;

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
						Vector3 effectPos = battleCharacter_->transform.position + (battleCharacter_->GetStateMachine()->GetMoveDirection());
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
						// 通常攻撃と同じようにキャラクターの向きを取得
						Vector3 forward = Vector3(0.0f, 0.0f, 1.0f);
						battleCharacter_->transform.localRotation.Apply(forward);
						forward.y = 0.0f;
						forward.Normalize();

						// 向きに合わせて回転を設定
						Quaternion effectRot = Quaternion::Identity;
						if (forward.LengthSq() > 0.0001f)
						{
							float yaw = atan2f(forward.x, forward.z) + Math::PI / 2.0f;
							effectRot.SetRotationY(yaw);

							Quaternion tilt;
							tilt.SetRotationX(Math::DegToRad(-45.0f));
							effectRot = effectRot * tilt;
						}

						{
							Vector3 effectPos = battleCharacter_->transform.position + (forward * 30.0f);
							effectPos.y += 30.0f;

							effectManagerObject_->PlayEffect(
								enEffectKind_PlayerAttackCharge_Slash,
								effectPos,
								effectRot,
								Vector3::One * 1.2f
							);
						}

						{
							// mixamorig:Spine1 ボーン座標を取得
							Vector3 effectPos = battleCharacter_->transform.position;
							effectPos.y += 30.0f;

							auto* model = battleCharacter_->GetStateMachine()->GetModelRender();
							if (model)
							{
								auto& skeleton = model->GetSkeleton();
								int boneId = skeleton.FindBoneID(L"mixamorig:Spine1");
								if (boneId != -1)
								{
									Quaternion boneRot;
									Vector3 boneScale;
									skeleton.GetBone(boneId)->CalcWorldTRS(effectPos, boneRot, boneScale);
								}
							}

							effectManagerObject_->PlayEffect(
								enEffectKind_PlayerAttackCharge_End,
								effectPos,
								Quaternion::Identity,
								Vector3::One * 1.5f
							);
						}
					}

					// ノックバックエフェクトの再生判定
					if (battleCharacter_->GetStateMachine()->CheckAndConsumeKnockBack())
					{
						Vector3 effectPos = battleCharacter_->transform.position + (battleCharacter_->GetStateMachine()->GetMoveDirection() * 30.0f);
						effectPos.y += 30.0f;

						effectManagerObject_->PlayEffect(
							enEffectKind_PlayerKnockBack,
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
					}

				if (!isTutorialMode_ || tutorialEnemyMoveEnabled_)
				{
					/** ストーンの攻撃判定 */
					for (auto* stone : stoneEventCharacters_)
					{
						if (!stone) continue;

						bool hit = stone->GetStateMachine()->CheckAndConsumeAttackGhostCreated();
						if (hit)
						{
							/** 無敵中はダメージを受けない */
							if (isInvincible_)
							{
								continue;
							}

							/** ガード中はダメージを受けない */
							if (battleCharacter_->GetStateMachine()->IsGuarding())
							{
								if (guardSuccessCooldown_ <= 0.0f)
								{
									NotifyGuardSucceeded();
									guardSuccessCooldown_ = 1.0f;
								}
								continue;
							}

							/** 回避中はダメージを受けない */
							if (battleCharacter_->GetStateMachine()->IsAvoiding())
							{
								continue;
							}

							float attackPower = stone->GetStatus()->GetAttackPower();
							if (!tutorialNoDamage_)
							{
								float newHp = battleCharacter_->GetStatus()->GetCurrentHp() - attackPower;
								newHp = max(newHp, 0.0f);
								battleCharacter_->GetStatus()->SetCurrentHp(newHp);
								if (newHp <= 0.0f)
								{
									battleCharacter_->GetStateMachine()->OnDead();
								}
							}

							/** ノックバック */
							battleCharacter_->GetStateMachine()->OnKnockBack();
							app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::DamagePlayer));

							/** 無敵時間開始 */
							isInvincible_ = true;
							invincibleTimer_ = INVINCIBLE_TIME;
							battleCharacter_->GetStateMachine()->SetInvincible(true);

							/** UIに無敵状態を通知 */
							if (playerHpUIObject_)
							{
								playerHpUIObject_->StartInvincible(INVINCIBLE_TIME);
							}
						}
					}

					/** マッシュルームの攻撃判定 */
					for (auto* mushroom : mushroomEventCharacters_)
					{
						if (!mushroom) continue;

						if (mushroom->GetStateMachine()->CheckAndConsumeAttackGhostCreated())
						{
							/** 無敵中はスキップ */
							if (isInvincible_)
							{
								continue;
							}

							/** ガード中はダメージを受けない */
							if (battleCharacter_->GetStateMachine()->IsGuarding())
							{
								if (guardSuccessCooldown_ <= 0.0f)
								{
									NotifyGuardSucceeded();
									guardSuccessCooldown_ = 1.0f;
								}
								continue;
							}

							/** 回避中はダメージを受けない */
							if (battleCharacter_->GetStateMachine()->IsAvoiding())
							{
								continue;
							}

							float attackPower = mushroom->GetStatus()->GetAttackPower();
							if (!tutorialNoDamage_)
							{
								float newHp = battleCharacter_->GetStatus()->GetCurrentHp() - attackPower;
								newHp = max(newHp, 0.0f);
								battleCharacter_->GetStatus()->SetCurrentHp(newHp);
								if (newHp <= 0.0f)
								{
									battleCharacter_->GetStateMachine()->OnDead();
								}
							}

							/** ノックバック */
							battleCharacter_->GetStateMachine()->OnKnockBack();
							app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::DamagePlayer));

							/** 無敵開始 */
							isInvincible_ = true;
							invincibleTimer_ = INVINCIBLE_TIME;
							battleCharacter_->GetStateMachine()->SetInvincible(true);
							if (playerHpUIObject_)
							{
								playerHpUIObject_->StartInvincible(INVINCIBLE_TIME);
							}
						}
					}
				} // if (!isTutorialMode_ || tutorialEnemyMoveEnabled_)

				// 衝突後の処理
				{
					for (auto& notify : notifyList_) {
						if (notify->ID() == DamageNotify::StaticID())
						{
							auto* dmg = static_cast<DamageNotify*>(notify.get());
							// defender がプレイヤー自身の場合はスキップ
							if (dmg->defender == battleCharacter_) continue;

							/** ダメージ計算・適用 */
							bool isCriticalHit = false;
							int damage = CalcDamage(dmg->attacker, dmg->defender, dmg->chargeLevel, &isCriticalHit);
							const float oldHp = dmg->defender->GetStatus()->GetCurrentHp();
							if (!tutorialNoDamage_)
							{
								float newHp = oldHp - damage;
								newHp = max(newHp, 0.0f);
								dmg->defender->GetStatus()->SetCurrentHp(newHp);
								dmg->defender->TakeDamage(damage);
							}

							/** 溜め攻撃ヒット時：プレイヤー側もヒットストップ（段階0含む） */
							if (dmg->isBlowBack)
							{
								float hitStopDuration = 0.0f;
								if (const auto* p = app::core::ParameterManager::Get().GetParameter<app::core::MasterBattleCharacterParameter>())
								{
									hitStopDuration = dmg->chargeLevel <= 0 ? p->hitStopDurationSmall
									                : dmg->chargeLevel == 1 ? p->hitStopDurationMedium
									                                        : p->hitStopDurationLarge;
								}
								battleCharacter_->GetStateMachine()->StartHitStop(hitStopDuration);
							}

							/** カメラシェイク＆FOV */
							if (auto* gameCamera = gameCameraController_->As<app::camera::GameCamera>())
							{
								if (dmg->chargeLevel > 0)
								{
									/** 溜め攻撃：Lv1=Small / Lv2=Medium / Lv3=Large */
									const auto size = static_cast<app::camera::ShakeSize>(min(dmg->chargeLevel - 1, 2));
									gameCamera->StartShake(size);

									/** 溜め攻撃がヒットすると、すべてのレベルにおいて視野角が55度に */
									const int fovIndex = min(dmg->chargeLevel - 1, CHARGE_HIT_FOV_MAX_INDEX);
									const float targetFov = CHARGE_HIT_FOV_PRESETS[fovIndex].fov;
									gameCamera->OnAttackHit(
										targetFov,
										CHARGE_HIT_FOV_FADE_IN,
										CHARGE_HIT_FOV_FADE_OUT,
										CHARGE_HIT_FOV_IDLE_THRESHOLD);
								}
								else if (dmg->comboIndex == 1)
								{
									/** 通常2コンボ目：上方向への打ち上げ（Small） */
									gameCamera->StartShakeUpward(app::camera::ShakeSize::Small);
								}
								else
								{
									/** 通常攻撃（1・3コンボ目） */
									gameCamera->StartShake(app::camera::ShakeSize::Small);
								}
							}

							/** ダメージポップ通知 */
							if (damagePopListener_ && oldHp > 0.0f)
							{
								damagePopListener_->OnDamageDealt(damage, dmg->defender->transform.position, isCriticalHit);
							}

							if (dmg->enemyType == DamageNotify::EnemyType::Stone)
							{
								// ノックバック
								auto* enemy = static_cast<app::actor::StoneEventCharacter*>(dmg->defender);
								enemy->GetStateMachine()->OnKnockBack(dmg->knockBackDirection, dmg->isBlowBack, dmg->chargeLevel);
								// 死亡判定
								if (!tutorialNoDamage_ && dmg->defender->GetStatus()->GetCurrentHp() <= 0)
								{
									enemy->GetStateMachine()->OnDead();
								}
							}
							else if (dmg->enemyType == DamageNotify::EnemyType::Mushroom)
							{
								// ノックバック
								auto* enemy = static_cast<app::actor::MushroomEventCharacter*>(dmg->defender);
								enemy->GetStateMachine()->OnKnockBack(dmg->knockBackDirection, dmg->isBlowBack, dmg->chargeLevel);
								// 死亡判定
								if (!tutorialNoDamage_ && dmg->defender->GetStatus()->GetCurrentHp() <= 0)
								{
									enemy->GetStateMachine()->OnDead();
								}
							}
						}
						// 敵→プレイヤーへのダメージ
						else if (notify->ID() == PlayerDamageNotify::StaticID())
						{
							auto* dmg = static_cast<PlayerDamageNotify*>(notify.get());

							/** ガード中はダメージを受けない */
							if (battleCharacter_->GetStateMachine()->IsGuarding())
							{
								if (guardSuccessCooldown_ <= 0.0f)
								{
									NotifyGuardSucceeded();
									guardSuccessCooldown_ = 1.0f;
								}
								continue;
							}

							/** 回避中はダメージを受けない */
							if (battleCharacter_->GetStateMachine()->IsAvoiding())
							{
								continue;
							}

							float attackPower = dmg->attacker->GetStatus()->GetAttackPower();
							if (!tutorialNoDamage_)
							{
								float newHp = battleCharacter_->GetStatus()->GetCurrentHp() - attackPower;
								newHp = max(newHp, 0.0f);
								battleCharacter_->GetStatus()->SetCurrentHp(newHp);
								if (newHp <= 0.0f)
								{
									battleCharacter_->GetStateMachine()->OnDead();
								}
							}
						}
					}
					notifyList_.clear();
				}
			}

				if (auto* gameCamera = gameCameraController_->As<app::camera::GameCamera>())
				{
					// 溜め中（ボタン押下中）のみFOVを狭める。
					// GetChargeLevel() > 0 は振り下ろし中（End フェーズ）なので呼ばない
					auto* playerSM = battleCharacter_->GetStateMachine();
					if (playerSM->IsChargeAttacking() && playerSM->GetChargeLevel() == 0)
						gameCamera->OnCharging(playerSM->GetCurrentChargingLevel());

					auto cameraData = gameCamera->GetCameraData();
					cameraSteering_->Update(cameraData, g_gameTime->GetFrameDeltaTime());
					gameCamera->SetState(cameraData);
				}

				/** 制限時間の管理 */
				{
					if (!timerUIObject_) return;
					// 残り時間が必要な取得
					remainTime_ = timerUIObject_->GetTimer();

					// 残り5〜1秒のカウントダウン表示
					if (!timeUpTriggered_ && battleSequenceObject_)
					{
						float remaining = timerUIObject_->GetTimer();
						if (remaining > 0.0f && remaining < 5.5f)
						{
							int countDown = static_cast<int>(remaining) + 1;
							if (countDown >= 1 && countDown <= 5 && countDown != lastCountShown_)
							{
								lastCountShown_ = countDown;
								battleSequenceObject_->ShowTimeUpCountdown(countDown);
							}
						}
					}

					if (timerUIObject_->IsTimeUp() && battleSequenceObject_ && !timeUpTriggered_)
					{
						timeUpTriggered_ = true;
						timeUpFreezeFrames_ = kTimeUpFreezeFrameCount;
						g_gameTime->EnableFixedFrameDeltaTime(0.0001f);
					}
				}

				/** レベル */
				{
					// ゲージ折り返しを検知したらキャラのLv.を上げる
					if (playerHpUIObject_ && playerHpUIObject_->IsLevelUp())
					{
						if (battleCharacter_) {
							battleCharacter_->LevelUp();
							if (isTutorialMode_) NotifyTutorialLevelUp();
							// レベルアップエフェクト再生
							if (effectManagerObject_)
							{
								effectManagerObject_->PlayEffectFollow(
									enEffectKind_PlayerSpawn,
									&battleCharacter_->transform.position,
									Quaternion::Identity,
									Vector3::One
								);
							}

						}
						playerHpUIObject_->ClearLevelUp();  // フラグをリセット
					}
				}

			}
		}


		void BattleManager::SetPause(bool isPause)
		{
			isPause_ = isPause;
			/** プレイヤー停止 */
			if (battleCharacter_)
			{
				battleCharacter_->SetPouse(isPause_);
			}
			/** ストーンモンスター停止 */
			for (auto* stone : stoneEventCharacters_)
			{
				if (stone) { stone->SetPause(isPause_); }
			}
			/** きのこモンスター停止 */
			for (auto* mushroom : mushroomEventCharacters_)
			{
				if (mushroom) { mushroom->SetPause(isPause_); }
			}
			/** スポーンマネージャーのポーズ設定 */
			if (eventCharacterSpawnManagerObject_)
			{
				eventCharacterSpawnManagerObject_->SetPause(isPause_);
			}
			/** カウントダウン停止 */
			if (timerUIObject_)
			{
				if (isPause_)
				{
					timerUIObject_->StopTimer();
				}
				else
				{
					timerUIObject_->StartTimer();
				}
			}
		}

		bool BattleManager::IsTimeUpFinished() const
		{
			return battleSequenceObject_ && battleSequenceObject_->IsTimeUpFinished();
		}

		int BattleManager::CalcDamage(const app::actor::BattleCharacter* attacker, const app::actor::Character* defender, int chargeLevel, bool* outIsCritical) const
		{
			float atk = attacker->GetTotalAttack();

			const auto* param = app::core::ParameterManager::Get().GetParameter<app::core::MasterBattleCharacterParameter>();

			if (chargeLevel > 0 && param)
			{
				if (chargeLevel == 1)      atk *= param->chargeAttackMultiplierLevel1;
				else if (chargeLevel == 2) atk *= param->chargeAttackMultiplierLevel2;
				else                       atk *= param->chargeAttackMultiplier;
			}

			// クリティカル判定
			bool isCritical = false;
			if (param)
			{
				float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				if (r < param->criticalRate)
				{
					atk *= param->criticalMultiplier;
					isCritical = true;
				}
			}
			if (outIsCritical) *outIsCritical = isCritical;

			float def = defender->GetTotalDefensePower();
			float damage = atk - def;
			return static_cast<int>(damage > 0.0f ? roundf(damage) : 0.0f);
		}


		void BattleManager::TriggerTBDRSpawnLight(const Vector3& pos, const Vector3& color, float range, float duration)
		{
			TBDRSpawnLightEntry entry;
			entry.position  = pos;
			entry.peakColor = color;
			entry.range     = range;
			entry.timer     = duration;
			entry.duration  = duration;
			tbdrSpawnLights_.push_back(entry);
		}


		void BattleManager::UpdateTBDRSpawnLights()
		{
			const float dt = g_gameTime->GetFrameDeltaTime();
			int idx = 0;

			// ---- 松明ライト（壁沿いに 8本）----
			// TBDR が無効のときはスキップ（スポーンライトは常に動く）
			if (torchLightsEnabled_ && g_renderingEngine->IsTBDREnabled() && battleCharacter_ != nullptr)
			{
				const float HEIGHT = battleCharacter_->transform.position.y + 80.f;
				constexpr float RADIUS = 650.f;
				constexpr float RANGE  = 200.f; // √(80²+250²)≈263 より小 → 中心に届かない
				for (int i = 0; i < 16; i++)
				{
					if (idx >= MAX_TBDR_POINT_LIGHT) break;
					const float angle = (3.14159265f * 2.f / 16.f) * static_cast<float>(i);
					auto& lig = g_sceneLight->GetTBDRPointLight(idx++);
					lig.position = Vector3(sinf(angle) * RADIUS, HEIGHT, cosf(angle) * RADIUS);
					lig.color    = Vector3(1.0f, 0.75f, 0.1f);
					lig.range    = RANGE;
				}
			}

			// スポーンライト（動的・フェードアウト）
			for (auto it = tbdrSpawnLights_.begin(); it != tbdrSpawnLights_.end(); )
			{
				it->timer -= dt;
				if (it->timer <= 0.0f)
					it = tbdrSpawnLights_.erase(it);
				else
					++it;
			}
			for (auto& e : tbdrSpawnLights_)
			{
				if (idx >= MAX_TBDR_POINT_LIGHT) break;
				const float t    = e.timer / e.duration;
				auto& lig        = g_sceneLight->GetTBDRPointLight(idx++);
				lig.position     = e.position;
				lig.color        = e.peakColor * t;
				lig.range        = e.range;
			}

			g_sceneLight->SetNumTBDRPointLights(idx);
		}


		void BattleManager::AddPlayerGauge(int amount)
		{
			if (playerHpUIObject_)
			{
				playerHpUIObject_->AddLevelUpGauge(amount);
				app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::GaugeUp));
			}
		}


		void BattleManager::Render(RenderContext& rc)
		{
			if (layout_)
			{
				layout_->Render(rc);
			}
			if (app::effect::SwordDecalManager::IsAvailable())
			{
				app::effect::SwordDecalManager::Get().Render(rc);
			}
		}


		bool BattleManager::IsOpeningSequenceDone() const
		{
			if (isTutorialMode_) return true;
			return battleSequenceObject_ && battleSequenceObject_->IsOpeningDone();
		}


		void BattleManager::SetTutorialEnemyMoveEnabled(bool enabled)
		{
			tutorialEnemyMoveEnabled_ = enabled;
			for (auto* stone : stoneEventCharacters_)
			{
				if (stone) stone->GetStateMachine()->SetAIEnabled(enabled);
			}
			for (auto* mushroom : mushroomEventCharacters_)
			{
				if (mushroom) mushroom->GetStateMachine()->SetAIEnabled(enabled);
			}
		}


		bool BattleManager::IsTutorialAllEnemiesDefeated() const
		{
			if (!isTutorialMode_ || tutorialNeedsSpawn_) return false;
			if (!IsOpeningSequenceDone()) return false;
			if (!eventCharacterSpawnManagerObject_) return false;
			return eventCharacterSpawnManagerObject_->GetManager().GetActiveEnemyCount() == 0;
		}

		int BattleManager::GetTutorialActiveEnemyCount() const
		{
			if (!eventCharacterSpawnManagerObject_) return 0;
			return eventCharacterSpawnManagerObject_->GetManager().GetActiveEnemyCount();
		}

		bool BattleManager::IsPlayerDead() const
		{
			return battleCharacter_ && battleCharacter_->GetStateMachine()->IsDeadTriggered();
		}

		void BattleManager::SetGameOverFreeze(bool v)
		{
			gameOverFreeze_ = v;
			playerInputEnabled_ = !v;

			// 敵をポーズ（プレイヤーのDeadアニメは止めない）
			for (auto* stone : stoneEventCharacters_)
				if (stone) stone->SetPause(v);
			for (auto* mushroom : mushroomEventCharacters_)
				if (mushroom) mushroom->SetPause(v);
			if (eventCharacterSpawnManagerObject_)
				eventCharacterSpawnManagerObject_->SetPause(v);

			// タイマーを止める
			if (timerUIObject_)
			{
				if (v) timerUIObject_->StopTimer();
				else   timerUIObject_->StartTimer();
			}
		}


		void BattleManager::SetHpBarPreBlurRender(bool v)
		{
			if (playerHpUIObject_)
				playerHpUIObject_->SetPreBlurRender(v);
			if (app::actor::EventCharacterSpawnManager::IsAvailable())
				app::actor::EventCharacterSpawnManager::Get().SetAllHpBarsPreBlurRender(v);
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
					p.hp = json["hp"].get<float>();
					p.attackPower = json["attackPower"].get<float>();
					p.chargeAttackMultiplierLevel1 = json["chargeAttackMultiplierLevel1"].get<float>();
					p.chargeAttackMultiplierLevel2 = json["chargeAttackMultiplierLevel2"].get<float>();
					p.chargeAttackMultiplier = json["chargeAttackMultiplier"].get<float>();
					p.criticalRate = json["criticalRate"].get<float>();
					p.criticalMultiplier = json["criticalMultiplier"].get<float>();
					p.hitStopDurationSmall  = json["hitStopDurationSmall"].get<float>();
					p.hitStopDurationMedium = json["hitStopDurationMedium"].get<float>();
					p.hitStopDurationLarge  = json["hitStopDurationLarge"].get<float>();
					p.spawnLightColorR   = json["spawnLightColorR"].get<float>();
					p.spawnLightColorG   = json["spawnLightColorG"].get<float>();
					p.spawnLightColorB   = json["spawnLightColorB"].get<float>();
					p.spawnLightRange    = json["spawnLightRange"].get<float>();
					p.spawnLightDuration = json["spawnLightDuration"].get<float>();
				});
			// 武器パラメーター読み込み
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterWeaponParameter>(MASTER_WEAPON_PARAM_PATH,[](const nlohmann::json& json, app::core::MasterWeaponParameter& p)
				{
					p.attackPower = json["attackPower"].get<float>();
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
			// ストーンイベントキャラクターパラメータ読み込み
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterStoneEventCharacterParameter>(MASTER_STONE_EVENT_CHARACTER_PARAM_PATH, [](const nlohmann::json& json, app::core::MasterStoneEventCharacterParameter& p)
				{
					p.moveSpeed = json["moveSpeed"].get<float>();
					p.jumpMoveSpeed = json["jumpMoveSpeed"].get<float>();
					p.jumpPower = json["jumpPower"].get<float>();
					p.radius = json["radius"].get<float>();
					p.height = json["height"].get<float>();
					p.hp = json["hp"].get<float>();
					p.hitStopDurationSmall  = json["hitStopDurationSmall"].get<float>();
					p.hitStopDurationMedium = json["hitStopDurationMedium"].get<float>();
					p.hitStopDurationLarge  = json["hitStopDurationLarge"].get<float>();
					p.spawnLightColorR = json["spawnLightColorR"].get<float>();
					p.spawnLightColorG = json["spawnLightColorG"].get<float>();
					p.spawnLightColorB = json["spawnLightColorB"].get<float>();
					p.phases.clear();
					for (const auto& phase : json["phases"])
					{
						app::core::EnemyPhaseParameter pp;
						pp.requiredPlayerLevel = phase["requiredPlayerLevel"].get<int>();
						pp.attackPower = phase["attackPower"].get<float>();
						p.phases.push_back(pp);
					}
				});
			// マッシュルイベントキャラクターパラメータ読み込み
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterMushroomEventCharacterParameter>(MASTER_MUSHROOM_EVENT_CHARACTER_PARAM_PATH, [](const nlohmann::json& json, app::core::MasterMushroomEventCharacterParameter& p)
				{
					p.moveSpeed = json["moveSpeed"].get<float>();
					p.jumpMoveSpeed = json["jumpMoveSpeed"].get<float>();
					p.jumpPower = json["jumpPower"].get<float>();
					p.radius = json["radius"].get<float>();
					p.height = json["height"].get<float>();
					p.hp = json["hp"].get<float>();
					p.hitStopDurationSmall  = json["hitStopDurationSmall"].get<float>();
					p.hitStopDurationMedium = json["hitStopDurationMedium"].get<float>();
					p.hitStopDurationLarge  = json["hitStopDurationLarge"].get<float>();
					p.spawnLightColorR = json["spawnLightColorR"].get<float>();
					p.spawnLightColorG = json["spawnLightColorG"].get<float>();
					p.spawnLightColorB = json["spawnLightColorB"].get<float>();
					p.phases.clear();
					for (const auto& phase : json["phases"])
					{
						app::core::EnemyPhaseParameter pp;
						pp.requiredPlayerLevel = phase["requiredPlayerLevel"].get<int>();
						pp.attackPower = phase["attackPower"].get<float>();
						p.phases.push_back(pp);
					}
				});
		}
	}
}
