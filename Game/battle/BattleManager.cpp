/**
 * BattleManager.cpp
 * バトル管理
 */
#include "stdafx.h"
#include "BattleManager.h"
#include <chrono>
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
#include "actor/EnemyPhase.h"
#include "sound/SoundManager.h"
#include "core/GameResultData.h"
#include "ui/PhaseUI.h"


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

	/** 溜め攻撃命中時の視野角 */
	struct ChargeHitFovPreset
	{
		float fov;
	};

	/** 溜め攻撃用のFOVプリセット */
	constexpr ChargeHitFovPreset CHARGE_HIT_FOV_PRESETS[] = {
		{ 55.0f },
		{ 55.0f },
		{ 55.0f },
	};
	/** ヒット時FOV縮小時間（ズームイン） */
	constexpr float CHARGE_HIT_FOV_FADE_IN = 0.15f;
	/** ヒット時FOV復帰時間（ズームアウト） */
	constexpr float CHARGE_HIT_FOV_FADE_OUT = 0.12f;
	/** 静止状態判定のしきい値 */
	constexpr float CHARGE_HIT_FOV_IDLE_THRESHOLD = 0.01f;
	/** プリセットの最大インデックス */
	constexpr int CHARGE_HIT_FOV_MAX_INDEX = 2;
    /** タイムアップ時に静止させる合計フレーム数定数 */
    static constexpr int TIME_UP_FREEZE_FRAME_COUNT = 10;

	/** プレイヤー用 */
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
	/** 敵用 */
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
		BattleManager* BattleManager::instance_ = nullptr;


		void BattleManager::PreloadCharacterAssets(app::actor::CharacterInitializeParameter& param)
		{
			param.Load();
			if (!param.modelName) return;

			auto& rm = app::resource::ResourceManager::GetInstance();

			rm.Load<app::resource::TkmResource>(param.modelName);

			std::string tksPath = param.modelName;
			const auto pos = tksPath.rfind(".tkm");
			if (pos != std::string::npos)
			{
				tksPath.replace(pos, 4, ".tks");
				rm.Load<app::resource::TksResource>(tksPath);
			}

			for (uint32_t i = 0; i < static_cast<uint32_t>(param.animationDataList.size()); ++i)
			{
				if (param.animationDataList[i].filename)
					rm.Load<app::resource::TkaResource>(param.animationDataList[i].filename);
			}
		}


		BattleManager::BattleManager()
		{
			app::effect::SwordDecalManager::Initialize();
			app::gimmick::WarpSystem::Initialize();
			app::collision::CollisionHitManager::Initialize();
			app::collision::GhostBodyManager::Get().RegisterCallback([](app::collision::GhostBody* a, app::collision::GhostBody* b)
				{
					/** 衝突ペア登録 */
					app::collision::CollisionHitManager::Get().RegisterHitPair(a, b);
				});
		}


		BattleManager::~BattleManager()
		{
			/** リザルト用データを保存。playerHpUIObject_ 削除前に取得する。 */
			{
				auto& result = app::GameResultData::Get();
				result.Reset();
				result.level = playerHpUIObject_ ? playerHpUIObject_->GetLevel() : 0;
				result.stoneKillCount = stoneKillCount_;
				result.mushroomKillCount = mushroomKillCount_;
			}

			/** スポーン済み敵とHPバーを先にクリーンアップ */
			if (eventCharacterSpawnManagerObject_)
			{
				eventCharacterSpawnManagerObject_->GetManager().CleanUp();
			}

			/** 溜め攻撃中などステート中断で放置される攻撃ゴースト・エフェクトを確実に片付ける */
			if (battleCharacter_ && battleCharacter_->GetStateMachine())
			{
				battleCharacter_->GetStateMachine()->ForceExitCurrentState();
			}

			DeleteGO(skyCube_);
			DeleteGO(moon_);
			DeleteGO(battleCharacter_);
			DeleteGO(battleSequenceObject_);
			DeleteGO(effectManagerObject_);
			DeleteGO(effectManager2DObject_);
			DeleteGO(pauseManagerObject_);
			DeleteGO(eventCharacterSpawnManagerObject_);
			DeleteGO(timerUIObject_);
			DeleteGO(playerHpUIObject_);
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

			/** パラメーター解放 */
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
			/** DebugScene 用の同期版。BattleScene/TutorialScene は LoadStep()使用 */
			while (!LoadStep()) {}
		}


		bool BattleManager::LoadStep()
		{
			switch (loadStep_++)
			{
			case 0:
				LoadParameter();
				PreloadCharacterAssets(sPlayerInitializeParameter);
				PreloadCharacterAssets(sStoneEnemyInitializeParameter);
				PreloadCharacterAssets(sMushroomEnemyInitializeParameter);
				{
					auto& rm = app::resource::ResourceManager::GetInstance();
					rm.Load<app::resource::TkmResource>("Assets/ModelData/stage/stage.tkm");
					rm.Load<app::resource::TkmResource>("Assets/ModelData/stage/moon.tkm");
					rm.Load<app::resource::TkmResource>("Assets/ModelData/preset/sky.tkm");
					/** デカールモデル: SwordDecalManager が初回使用時に同期ロードするのを防ぐ */
					rm.Load<app::resource::TkmResource>("Assets/ModelData/decal/atkDefaultDecal.tkm");
					rm.Load<app::resource::TkmResource>("Assets/ModelData/decal/stoneDebrisScar.tkm");
					rm.Load<app::resource::TkmResource>("Assets/ModelData/decal/mushroomDebrisScar.tkm");
					rm.Load<app::resource::TkmResource>("Assets/ModelData/decal/atkChargeLv1.tkm");
					rm.Load<app::resource::TkmResource>("Assets/ModelData/decal/atkChargeLv2.tkm");
					rm.Load<app::resource::TkmResource>("Assets/ModelData/decal/atkChargeLv3.tkm");
					rm.Load<app::resource::TkmResource>("Assets/ModelData/decal/atkChargeLv1_Floor.tkm");
					rm.Load<app::resource::TkmResource>("Assets/ModelData/decal/atkChargeLv2_Floor.tkm");
					rm.Load<app::resource::TkmResource>("Assets/ModelData/decal/atkChargeLv3_Floor.tkm");
					/**
                     * UI用DDSをOSページキャッシュに先読み（ワーカースレッド側）。
                     * メインスレッドがTexture::InitFromDDSFile()を呼ぶ時点でRAMから読める。
                     */ 
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/numbers/numbers.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/numbers/0.dds");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/numbers/1.dds");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/numbers/2.dds");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/numbers/3.dds");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/numbers/4.dds");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/numbers/5.dds");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/numbers/6.dds");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/numbers/7.dds");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/numbers/8.dds");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/numbers/9.dds");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/word/PHASE.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/timer/clock_hand.dds");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/hp/playerIcon.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/hp/whiteBackGround.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/hp/backGroundHP.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/hp/damageHP.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/LevelUp/currentLevel.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/LevelUp/LvIcon.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/levelUp/outerBar_Blue.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/levelUp/innerBar_Blue.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/levelUp/outerBar_Orange.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/levelUp/innerBar_Orange.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/levelUp/ATK POWER UP_Default.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/levelUp/ATK POWER UP_Bloom.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/levelUp/LEVEL UP_Default.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ui/levelUp/LEVEL UP_Bloom.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/preset/skyCubeMapNight_Toon_02.dds");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/player/maria_diffuse.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/player/maria_diffuse2.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/player/maria_normal.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/player/maria_specular.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/stage/brick10.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/stage/brick10_normal.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/stage/Stadium_ground.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/stage/Wall_Red.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/stage/ground_normal.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/stage/lroc_color_poles_4k.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/stage/moon_normal.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/enemy/stone/StoneMonster_1.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/enemy/mushroom/Albedo.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/enemy/mushroom/Metallic-Smoothness.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/decal/atkChargeLv1.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/decal/atkChargeLv2.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/decal/atkChargeLv3.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/decal/mushroomDebrisScar.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/decal/stoneDebrisScar.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/preset/NullAlbedoMap.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/preset/NullNormalMap.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/preset/specMap_None.DDS");
					rm.Load<app::resource::DdsWarmResource>("Assets/ModelData/preset/ZeroValueMap.DDS");
				}
				/**
                 * エフェクトマネージャーを先行生成。Start()（efkファイルの同期ロード）が
                 * 次フレームで走るため、ワーカースレッドの tkm/tka/tks ロードと重なり合う。
                 */ 
				effectManagerObject_   = NewGO<EffectManagerObject>(static_cast<uint8_t>(ObjectPriority::Default));
				effectManager2DObject_ = NewGO<EffectManager2DObject>(static_cast<uint8_t>(ObjectPriority::Default));
				return false;

			case 1:
				/**
                 * ワーカースレッドのロードがすべて完了するまで待機。
                 * FinalizeCompleted() が毎フレーム呼ばれ、完了済みリソースをエンジンバンクに登録済み。
                 */ 
				if (!app::resource::ResourceManager::GetInstance().IsIdle())
				{
					loadStep_--;
					return false;
				}
				return false;

			case 2:
				/**
                 * バックグラウンドスレッドで全テクスチャのGPUバッチアップロードを開始。
                 * メインスレッドはブロックされないためロード中もアニメーションが動き続ける。
                 */
				if (!texturePreloadFuture_.valid())
				{
					const wchar_t* const kAllTextures[] = {
						/** UIテクスチャ */
						L"Assets/ui/numbers/numbers.DDS",
						L"Assets/ui/numbers/0.dds",
						L"Assets/ui/numbers/1.dds",
						L"Assets/ui/numbers/2.dds",
						L"Assets/ui/numbers/3.dds",
						L"Assets/ui/numbers/4.dds",
						L"Assets/ui/numbers/5.dds",
						L"Assets/ui/numbers/6.dds",
						L"Assets/ui/numbers/7.dds",
						L"Assets/ui/numbers/8.dds",
						L"Assets/ui/numbers/9.dds",
						L"Assets/ui/word/PHASE.DDS",
						L"Assets/ui/timer/clock_hand.dds",
						L"Assets/ui/hp/playerIcon.DDS",
						L"Assets/ui/hp/whiteBackGround.DDS",
						L"Assets/ui/hp/backGroundHP.DDS",
						L"Assets/ui/hp/damageHP.DDS",
						L"Assets/ui/LevelUp/currentLevel.DDS",
						L"Assets/ui/LevelUp/LvIcon.DDS",
						L"Assets/ui/levelUp/outerBar_Blue.DDS",
						L"Assets/ui/levelUp/innerBar_Blue.DDS",
						L"Assets/ui/levelUp/outerBar_Orange.DDS",
						L"Assets/ui/levelUp/innerBar_Orange.DDS",
						L"Assets/ui/levelUp/ATK POWER UP_Default.DDS",
						L"Assets/ui/levelUp/ATK POWER UP_Bloom.DDS",
						L"Assets/ui/levelUp/LEVEL UP_Default.DDS",
						L"Assets/ui/levelUp/LEVEL UP_Bloom.DDS",
						/** 3Dモデル・スカイ・デカールテクスチャ */
						L"Assets/ModelData/preset/skyCubeMapNight_Toon_02.dds",
						L"Assets/ModelData/player/maria_diffuse.DDS",
						L"Assets/ModelData/player/maria_diffuse2.DDS",
						L"Assets/ModelData/player/maria_normal.DDS",
						L"Assets/ModelData/player/maria_specular.DDS",
						L"Assets/ModelData/stage/brick10.DDS",
						L"Assets/ModelData/stage/brick10_normal.DDS",
						L"Assets/ModelData/stage/Stadium_ground.DDS",
						L"Assets/ModelData/stage/Wall_Red.DDS",
						L"Assets/ModelData/stage/ground_normal.DDS",
						L"Assets/ModelData/stage/lroc_color_poles_4k.DDS",
						L"Assets/ModelData/stage/moon_normal.DDS",
						L"Assets/ModelData/enemy/stone/StoneMonster_1.DDS",
						L"Assets/ModelData/enemy/mushroom/Albedo.DDS",
						L"Assets/ModelData/enemy/mushroom/Metallic-Smoothness.DDS",
						L"Assets/ModelData/decal/atkChargeLv1.DDS",
						L"Assets/ModelData/decal/atkChargeLv2.DDS",
						L"Assets/ModelData/decal/atkChargeLv3.DDS",
						L"Assets/ModelData/decal/mushroomDebrisScar.DDS",
						L"Assets/ModelData/decal/stoneDebrisScar.DDS",
						/** 全モデルマテリアルが使用するNull/プリセットテクスチャ（初回モデル初期化時のディスクI/Oを防ぐ） */
						L"Assets/ModelData/preset/NullAlbedoMap.DDS",
						L"Assets/ModelData/preset/NullNormalMap.DDS",
						L"Assets/ModelData/preset/specMap_None.DDS",
						L"Assets/ModelData/preset/ZeroValueMap.DDS",
						/** ゲームオーバーUIテクスチャ（遅延生成のGameOverSequenceがキャッシュヒットするよう先読み） */
						L"Assets/ui/gameOver/gameOver_Fog.DDS",
						L"Assets/ui/gameOver/gameOver_Word.DDS",
					};
					texturePreloadFuture_ = Texture::BatchPreloadToCacheAsync(
						kAllTextures,
						static_cast<int>(ARRAYSIZE(kAllTextures))
					);
					/**
                     * ケース3〜10 で使用するシェーダーをバックグラウンドスレッドで事前コンパイル。
                     * タイトルシーンに3Dモデルがないため、これがないとバトルロード中に
                     * model.fx 等が初めてコンパイルされてしまう。
                     */ 
					shaderPrecompileFuture_ = Shader::PrecompileAsync({
						/** skyCube.fx（ケース3） */
						{ "Assets/Shader/skyCube.fx",            "VSMain",              "vs_5_0" },
						{ "Assets/Shader/skyCube.fx",            "PSMain",              "ps_5_0" },
						/** model.fx フォワードパス（ケース4-5: プレイヤー + 敵） */
						{ "Assets/Shader/model.fx",              "VSMain",              "vs_5_0" },
						{ "Assets/Shader/model.fx",              "VSSkinMain",          "vs_5_0" },
						{ "Assets/Shader/model.fx",              "PSNormalMain",        "ps_5_0" },
						{ "Assets/Shader/model.fx",              "PSShadowReceverMain", "ps_5_0" },
						/** model_gbuffer.fx（G-Buffer、シャドウレシーバー） */
						{ "Assets/Shader/model_gbuffer.fx",      "VSMain",              "vs_5_0" },
						{ "Assets/Shader/model_gbuffer.fx",      "VSSkinMain",          "vs_5_0" },
						{ "Assets/Shader/model_gbuffer.fx",      "PSMain",              "ps_5_0" },
						/** model_gbuffer_ns.fx（G-Buffer、非シャドウレシーバー） */
						{ "Assets/Shader/model_gbuffer_ns.fx",   "VSMain",              "vs_5_0" },
						{ "Assets/Shader/model_gbuffer_ns.fx",   "VSSkinMain",          "vs_5_0" },
						{ "Assets/Shader/model_gbuffer_ns.fx",   "PSMain",              "ps_5_0" },
						/** drawShadowMap.fx（シャドウキャスター） */
						{ "Assets/Shader/drawShadowMap.fx",      "VSMain",              "vs_5_0" },
						{ "Assets/Shader/drawShadowMap.fx",      "VSSkinMain",          "vs_5_0" },
						{ "Assets/Shader/drawShadowMap.fx",      "PSMain",              "ps_5_0" },
						/** modelWall.fx（ケース6: ステージ）— model.fx と同様のPSスプリット */
						{ "Assets/Shader/modelWall.fx",          "VSMain",              "vs_5_0" },
						{ "Assets/Shader/modelWall.fx",          "VSSkinMain",          "vs_5_0" },
						{ "Assets/Shader/modelWall.fx",          "PSNormalMain",        "ps_5_0" },
						{ "Assets/Shader/modelWall.fx",          "PSShadowReceverMain", "ps_5_0" },
						/** model_gbuffer_wall.fx（ケース6: ステージG-Buffer） */
						{ "Assets/Shader/model_gbuffer_wall.fx", "VSMain",              "vs_5_0" },
						{ "Assets/Shader/model_gbuffer_wall.fx", "VSSkinMain",          "vs_5_0" },
						{ "Assets/Shader/model_gbuffer_wall.fx", "PSMain",              "ps_5_0" },
						/** model_enemy_fade.fx（ケース5: 敵、GBufferパス） */
						{ "Assets/Shader/model_enemy_fade.fx",   "VSMain",              "vs_5_0" },
						{ "Assets/Shader/model_enemy_fade.fx",   "VSSkinMain",          "vs_5_0" },
						{ "Assets/Shader/model_enemy_fade.fx",   "PSMain",              "ps_5_0" },
						/** hpBar.fx（ケース7-8: PlayerHpUI + EnemyHpUIレイアウト） */
						{ "Assets/Shader/hpBar.fx",              "VSMain",              "vs_5_0" },
						{ "Assets/Shader/hpBar.fx",              "PSMain",              "ps_5_0" },
						/** numberSprite.fx（DamagePopPool / UINumberSprite） */
						{ "Assets/Shader/numberSprite.fx",       "VSMain",              "vs_5_0" },
						{ "Assets/Shader/numberSprite.fx",       "PSMain",              "ps_5_0" },
					});
				}
				{
					bool texDone  = texturePreloadFuture_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
					bool shadDone = !shaderPrecompileFuture_.valid() ||
					                shaderPrecompileFuture_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
					if (!texDone || !shadDone)
					{
						loadStep_--;
						return false;
					}
				}
				return false;

			case 3:
				/** スカイキューブ + 月オブジェクト */
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

			case 4:
				/** スポーンマネージャー(エフェクトマネージャーは case 0 で作成済み) */
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
									stone->AddState<app::actor::StonePounceAttackState>();
								}
								stone->GetStatus()->SetFriction(stageParam->friction);
								stone->GetStatus()->SetGravity(stageParam->gravity);
								stone->GetCharacterController()->SetGravity(stageParam->gravity);
								if (isTutorialMode_)
								{
									stone->GetStateMachine()->SetAIEnabled(false);
									const auto* sp = app::core::ParameterManager::Get().GetParameter<app::core::MasterStoneEventCharacterParameter>();
									if (sp)
									{
										stone->GetStatus()->SetHp(sp->tutorialHp);
										stone->GetStatus()->SetCurrentHp(sp->tutorialHp);
										stone->SyncCurrentHPFromStatus();
										stone->GetStatus()->SetAttackPower(sp->tutorialAttackPower);
									}
								}
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
									mushroom->AddState<app::actor::MushroomPoisonCastState>();
								}
								mushroom->GetStatus()->SetFriction(stageParam->friction);
								mushroom->GetStatus()->SetGravity(stageParam->gravity);
								mushroom->GetCharacterController()->SetGravity(stageParam->gravity);
								if (isTutorialMode_)
								{
									mushroom->GetStateMachine()->SetAIEnabled(false);
									const auto* mp = app::core::ParameterManager::Get().GetParameter<app::core::MasterMushroomEventCharacterParameter>();
									if (mp)
									{
										mushroom->GetStatus()->SetHp(mp->tutorialHp);
										mushroom->GetStatus()->SetCurrentHp(mp->tutorialHp);
										mushroom->SyncCurrentHPFromStatus();
										mushroom->GetStatus()->SetAttackPower(mp->tutorialAttackPower);
									}
								}
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

			case 5:
				/** プレイヤー + キャラクターステアリング + スポーンマネージャー起動 */
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

			case 6:
				/** ステージギミック */
				{
					testGimmickList_.resize(1);
					testGimmickList_[0] = NewGO<app::actor::StaticGimmick>(static_cast<uint8_t>(ObjectPriority::Default), "testGimmick");
					testGimmickList_[0]->transform.position = Vector3(0.0f, -50.0f, 0.0f);
					testGimmickList_[0]->transform.scale = Vector3(5.0f, 5.0f, 5.0f);
					testGimmickList_[0]->Initialize("Assets/ModelData/stage/stage.tkm", "Assets/Shader/modelWall.fx", "Assets/Shader/model_gbuffer_wall.fx");
				}
				return false;

			case 7:
				/** タイマーUI */
				if (!isTutorialMode_)
				{
					timerUIObject_ = NewGO<app::ui::TimerUIObject>(static_cast<uint8_t>(ObjectPriority::Default));
					timerUIObject_->SetTimer(remainTime_);
				}
				return false;

			case 8:
				/** ダメージポップ */
				damagePopPool_ = new app::ui::DamagePopPool();
				damagePopPool_->Initialize();
				SetDamagePopListener(damagePopPool_);
				return false;

			case 9:
				/** フェーズUI */
				if (!isTutorialMode_)
				{
					phaseUI_ = NewGO<app::ui::PhaseUI>(static_cast<uint8_t>(ObjectPriority::Default));
				}
				return false;

			case 10:
				/** レベルアップUI */
				if (!isTutorialMode_)
				{
					levelUpObject_ = NewGO<app::ui::LevelUpUIObject>(static_cast<uint8_t>(ObjectPriority::PlayerUI));
				}
				return false;

			case 11:
				/** プレイヤーHPUI */
				playerHpUIObject_ = NewGO<app::ui::PlayerHpUIObject>(static_cast<uint8_t>(ObjectPriority::PlayerUI));
				return false;

			case 12:
				/** カメラ + ポーズ + BGM + UI依存設定 */
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
					if (playerHpUIObject_ && levelUpObject_)
					{
						playerHpUIObject_->SetLevelUpUIObject(levelUpObject_);
					}
					if (phaseUI_)
					{
						eventCharacterSpawnManagerObject_->GetManager().SetPhaseUI(phaseUI_);
					}
				}
				return true;

			default:
				return true;
			}
		}


		void BattleManager::Update()
		{
			/** タイムアップフリーズ更新（ポーズ・シーケンス状態に関係なく常に実行） */
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

			/** 剣痕デカールの寿命管理 */
			if (app::effect::SwordDecalManager::IsAvailable()) {
				app::effect::SwordDecalManager::Get().Update();
			}

			UpdateTBDRSpawnLights();

            /**
             * アニメーション初期化フレームを確保してからシーケンスを生成する
             * （生成直後に SetPause するとアニメが走らず T ポーズになるため）
             * チュートリアル用：EffectManager の初期化完了後に敵を固定配置
             */
			/** バトルシーケンス完了の瞬間にスポーンマネージャーを解放する */
			const bool isOpeningDoneNow = IsOpeningSequenceDone();
			if (!wasOpeningSequenceDone_ && isOpeningDoneNow)
			{
				if (eventCharacterSpawnManagerObject_)
					eventCharacterSpawnManagerObject_->GetManager().SetOpeningSequenceDone(true);
			}
			wasOpeningSequenceDone_ = isOpeningDoneNow;

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

			/** チュートリアル練習フェーズ前：全滅時にリスポーン */
			if (tutorialRespawnEnabled_ && IsOpeningSequenceDone() && !tutorialNeedsSpawn_)
			{
				if (stoneEventCharacters_.empty())
					eventCharacterSpawnManagerObject_->GetManager().SpawnFixed(
						app::actor::EnemyType::STONE, tutorialStoneSpawnPos_, tutorialEnemySpawnRot_);
				if (mushroomEventCharacters_.empty())
					eventCharacterSpawnManagerObject_->GetManager().SpawnFixed(
						app::actor::EnemyType::MUSHROOM, tutorialMushroomSpawnPos_, tutorialEnemySpawnRot_);
			}

			/** カウントダウン（スタート演出）、チュートリアルでは不要 */
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
			/** シーケンスが終わっていない間はポーズ・入力をすべて封印 */
			bool isSequence = battleSequenceObject_ && !battleSequenceObject_->IsFinished();
			/** キャラクターたちに適用するポーズ状態（手動ポーズ中、シーケンス中、またはチュートリアルフリーズ中ならポーズ） */
			bool targetPauseState = currentPause || isSequence || tutorialFreeze_;

			if (isPause_ != targetPauseState)
			{
				SetPause(targetPauseState);
			}

			/** シーケンス中は手動ポーズ（メニュー表示）を禁止 */
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

			/** プレイヤースポーン時のスポットライト（シーケンス終了後に発火） */
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
			/** デバッグ機能：LB1+Downでプレイヤーに1ダメージ */
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

			/** デバッグ機能：LB1+UpでタイムアップUIをトグル表示 */
			if (g_pad[0]->IsPress(enButtonLB1) && g_pad[0]->IsTrigger(enButtonUp))
			{
				if (battleSequenceObject_ && battleSequenceObject_->IsFinished())
				{
					battleSequenceObject_->DebugToggleTimeUp();
				}
			}

			/** デバッグ機能：LB1+Rightでタイマーをx秒増加、LB1+Leftで減少 */
			if (timerUIObject_)
			{
				constexpr static float DEBUG_TIMER_STOP = 5.0f;
				if (g_pad[0]->IsPress(enButtonLB1) && g_pad[0]->IsTrigger(enButtonRight))
				{
					timerUIObject_->AddTimer(+DEBUG_TIMER_STOP);
                    /** カウントダウン表示をリセット */
					lastCountShown_ = -1;
				}
				if (g_pad[0]->IsPress(enButtonLB1) && g_pad[0]->IsTrigger(enButtonLeft))
				{
					timerUIObject_->AddTimer(-DEBUG_TIMER_STOP);
				}
			}
#endif // APP_DEBUG

			if (!isSequence)
			{
				if (playerInputEnabled_)
					characterSteering_->Update();
				else if (battleCharacter_)
					battleCharacter_->GetStateMachine()->ClearInput();

				/** 衝突判定更新 */
				if (app::collision::GhostBodyManager::IsAvailable()) {
					app::collision::GhostBodyManager::Get().Update();
				}
				/** 衝突ヒット管理更新 */
				app::collision::CollisionHitManager::Get().Update();
				

				if (!isTutorialMode_ || tutorialEnemyMoveEnabled_)
				{
				/** ストーンの追従処理 */
				for (auto* stone : stoneEventCharacters_)
				{
					if (!stone) { continue; }
                    Vector3 playerPosition = battleCharacter_->transform.position;
					Vector3 stonePosition = stone->transform.position;
					Vector3 diffXZ_Stone(playerPosition.x - stonePosition.x, 0.0f, playerPosition.z - stonePosition.z);
					float diffStone = diffXZ_Stone.Length();

					if (diffStone < 800.0f)
					{
						Vector3 DirectionToPlayer = diffXZ_Stone;
						DirectionToPlayer.Normalize();

						Vector3 stoneForward = Vector3(0.0f, 0.0f, 1.0f);
						stone->transform.localRotation.Apply(stoneForward);

						/** ストーン敵の前方向 */
						Vector3 forwardXZ(stoneForward.x, 0.0f, stoneForward.z);
						forwardXZ.Normalize();

						/** 向きだけのベクトルとストーン敵の前方向で内積 */
						float dot = forwardXZ.Dot(DirectionToPlayer);

						/** 角度のしきい値と計算 */
						float halfFovDegree = 60.0f;
						float halfFovRadians = halfFovDegree * (Math::PI / 180);
						/** 判定用のしきい値となるコサイン値 */
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
                    Vector3 playerPosition = battleCharacter_->transform.position;
					Vector3 mushroomPosition = mushroom->transform.position;
					Vector3 diffXZ_Mushroom(playerPosition.x - mushroomPosition.x, 0.0f, playerPosition.z - mushroomPosition.z);
					float diffMushroom = diffXZ_Mushroom.Length();

					if (diffMushroom < 200.0f)
					{
						Vector3 DirectionToPlayer = diffXZ_Mushroom;
						DirectionToPlayer.Normalize();

						Vector3 mushroomForward = Vector3(0.0f, 0.0f, 1.0f);
						mushroom->transform.localRotation.Apply(mushroomForward);

						// マッシュルームの前方向
						Vector3 forwardXZ(mushroomForward.x, 0.0f, mushroomForward.z);
						forwardXZ.Normalize();

						// 向きだけのベクトルとマッシュルームの前方向で内積
						float dot = forwardXZ.Dot(DirectionToPlayer);

						// 角度のしきい値と計算
						float halfFovDegree = 60.0f;
						float halfFovRadians = halfFovDegree * (Math::PI / 180);

						// 判定用のしきい値となるコサイン値
						float threshold = std::cos(halfFovRadians);

						if (dot > threshold || tutorialEnemyMoveEnabled_)
						{
							mushroom->GetStateMachine()->OnChase(DirectionToPlayer, playerPosition);
						}
					}
				}
				}

				/** 無敵時間の更新 */
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

				/** プレイヤーの攻撃アクション */
				{
					if (battleCharacter_->GetStateMachine()->IsSlashEffect()
						&& !isWaitEffectPlay_)
					{
						/** プレイヤーのモデルが向いている方向を前方ベクトルとして取得 */
						Vector3 forward = Vector3(0.0f, 0.0f, 1.0f);
						battleCharacter_->transform.localRotation.Apply(forward);
						forward.y = 0.0f;
						forward.Normalize();

						/** 攻撃開始時の向きを固定 */
						reservedEffectDir_ = forward;

						/** 回転を固定 */
						if (reservedEffectDir_.LengthSq() > 0.0001f)
						{
							float yaw = atan2f(reservedEffectDir_.x, reservedEffectDir_.z) + Math::PI / 2.0f;
							reservedEffectRot_.SetRotationY(yaw);

							/** 斬撃アニメーションに合わせた傾きを追加 */ 
							Quaternion tilt;
							tilt.SetRotationX(Math::DegToRad(-45.0f));
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

					/** エフェクトの生成待ち状態ならタイマーを更新 */
					if (isWaitEffectPlay_)
					{
						/** 座標はプレイヤーに追従、向きは reservedEffectDir_（固定）でオフセット */
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

					/** チャージエフェクトの再生判定 */
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

					/** チャージ攻撃エフェクトの再生判定 */
					if (battleCharacter_->GetStateMachine()->CheckAndConsumeChargeAttackEffectRequest())
					{
						/** 通常攻撃と同じようにキャラクターの向きを取得 */
						Vector3 forward = Vector3(0.0f, 0.0f, 1.0f);
						battleCharacter_->transform.localRotation.Apply(forward);
						forward.y = 0.0f;
						forward.Normalize();

						/** 向きに合わせて回転を設定 */
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
							/**  mixamorig:Spine1 ボーン座標を取得 */
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

					/** ノックバックエフェクトの再生判定 */
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

					/** 防御エフェクトの再生判定 */
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
							/** 無敵中はダメージを受けない*/
							if (isInvincible_)
							{
								continue;
							}

							/** ガード中はダメージを受けない*/
							if (battleCharacter_->GetStateMachine()->IsGuarding())
							{
								if (guardSuccessCooldown_ <= 0.0f)
								{
									NotifyGuardSucceeded();
									guardSuccessCooldown_ = 1.0f;
								}
								continue;
							}

							/** 回避中はダメージを受けない*/
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

							/** ガード中はダメージを受けない*/
							if (battleCharacter_->GetStateMachine()->IsGuarding())
							{
								if (guardSuccessCooldown_ <= 0.0f)
								{
									NotifyGuardSucceeded();
									guardSuccessCooldown_ = 1.0f;
								}
								continue;
							}

							/** 回避中はダメージを受けない*/
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
					// 毒雲詠唱完了通知を受け取り、雲を設置する
					for (auto* mushroom : mushroomEventCharacters_)
					{
						if (!mushroom) continue;
						if (mushroom->GetStateMachine()->CheckAndConsumePoisonCastComplete())
						{
							PlacePoisonCloud(mushroom, mushroom->transform.position);
						}
					}
				} // if (!isTutorialMode_ || tutorialEnemyMoveEnabled_)

				// 毒雲の更新・ダメージ処理（チュートリアル中も継続）
				UpdatePoisonClouds();

				// 衝突後の処理
				/** 衝突後の処理 */
				{
					for (auto& notify : notifyList_) {
						if (notify->ID() == DamageNotify::StaticID())
						{
							auto* dmg = static_cast<DamageNotify*>(notify.get());
							/** defender がプレイヤー自身の場合はスキップ */
							if (dmg->defender == battleCharacter_) continue;

							/** ヒット判定が無効なきのこ敵はスキップ */
							if (dmg->enemyType == DamageNotify::EnemyType::Mushroom)
							{
								auto* enemy = static_cast<app::actor::MushroomEventCharacter*>(dmg->defender);
								if (!enemy->GetStateMachine()->IsReceiveDamageEnabled())
									continue;
							}

							/** ダメージ計算と適用 */
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

							/** 溜め攻撃でチェック時：プレイヤー側もヒットストップ（段階を含む） */
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

							/** カメラシェイクとFOV */
							if (auto* gameCamera = gameCameraController_->As<app::camera::GameCamera>())
							{
								if (dmg->chargeLevel > 0)
								{
								/** 溜め攻撃でLv1=Small / Lv2=Medium / Lv3=Large */
									const auto size = static_cast<app::camera::ShakeSize>(min(dmg->chargeLevel - 1, 2));
									gameCamera->StartShake(size);

								/** 溜め攻撃でヒットすると、すべてのレベルにおいて視野角が55度に */
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
								/** 通常2コンボ目（上方向への打ち上げ）（Small） */
									gameCamera->StartShakeUpward(app::camera::ShakeSize::Small);
								}
								else
								{
								/** 通常攻撃1・3コンボ目 */
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
                                /** ノックバック */
								auto* enemy = static_cast<app::actor::StoneEventCharacter*>(dmg->defender);
								enemy->GetStateMachine()->OnKnockBack(dmg->knockBackDirection, dmg->isBlowBack, dmg->chargeLevel);
                                /** 死亡判定 */
								if (!tutorialNoDamage_ && dmg->defender->GetStatus()->GetCurrentHp() <= 0)
								{
									enemy->GetStateMachine()->OnDead();
								}
							}
							else if (dmg->enemyType == DamageNotify::EnemyType::Mushroom)
							{
								/** ノックバック */
								auto* enemy = static_cast<app::actor::MushroomEventCharacter*>(dmg->defender);
								enemy->GetStateMachine()->OnKnockBack(dmg->knockBackDirection, dmg->isBlowBack, dmg->chargeLevel);
								/** 死亡判定 */
								if (!tutorialNoDamage_ && dmg->defender->GetStatus()->GetCurrentHp() <= 0)
								{
									enemy->GetStateMachine()->OnDead();
								}
							}
						}
						/** 敵→プレイヤーへのダメージ */
						else if (notify->ID() == PlayerDamageNotify::StaticID())
						{
							auto* dmg = static_cast<PlayerDamageNotify*>(notify.get());

							/** ガード中はダメージを受けない*/
							if (battleCharacter_->GetStateMachine()->IsGuarding())
							{
								if (guardSuccessCooldown_ <= 0.0f)
								{
									NotifyGuardSucceeded();
									guardSuccessCooldown_ = 1.0f;
								}
								continue;
							}

							/** 回避中はダメージを受けない*/
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
                    /**
                     * 溜め中のボタン押下中のみ、FOVを狭める
                     * GetChargeLevel() > 0 は振り下ろし中（End フェーズ）なので呼ばない
                     */
					auto* playerSM = battleCharacter_->GetStateMachine();
					if (playerSM->IsChargeAttacking() && playerSM->GetChargeLevel() == 0)
						gameCamera->OnCharging(playerSM->GetCurrentChargingLevel());

					auto cameraData = gameCamera->GetCameraData();
					cameraSteering_->SetInputEnabled(playerInputEnabled_);
					cameraSteering_->Update(cameraData, g_gameTime->GetFrameDeltaTime());
					gameCamera->SetState(cameraData);
				}

				/** 制限時間の管理 */
				{
					if (!timerUIObject_) return;
					/** 最新の残り時間を取得 */
					remainTime_ = timerUIObject_->GetTimer();

					/** 残り5秒のカウントダウン表示 */
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
						timeUpFreezeFrames_ = TIME_UP_FREEZE_FRAME_COUNT;
						g_gameTime->EnableFixedFrameDeltaTime(0.0001f);
					}
				}

				/** レベル */
				{
					/** ゲージ折り返しを検知したらキャラのLv.を上げる */
					if (playerHpUIObject_ && playerHpUIObject_->IsLevelUp())
					{
						if (battleCharacter_) {
							battleCharacter_->LevelUp();
							if (isTutorialMode_) NotifyTutorialLevelUp();
							/** レベルアップエフェクトの再生 */
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
                    /** フラグリセット */
					playerHpUIObject_->ClearLevelUp();
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

		void BattleManager::PlacePoisonCloud(app::actor::MushroomEventCharacter* mushroom, const Vector3& position)
		{
			// 最大設置数を超えたら最も古いものを削除（煙エフェクトを明示的に停止）
			if (static_cast<int>(activePoisonClouds_.size()) >= PoisonCloud::MAX_COUNT)
			{
				auto& oldest = activePoisonClouds_.front();
				if (EffectManager::IsAvailable())
				{
					if (oldest.sporeHandle != INVALID_EFFECT_HANDLE)
					{
						EffectManager::Get().StopEffect(oldest.sporeHandle);
					}
					if (oldest.smokeHandle != INVALID_EFFECT_HANDLE)
					{
						EffectManager::Get().StopEffectRoot(oldest.smokeHandle);
					}
					for (EffectHandle bubbleHandle : oldest.bubbleHandles)
					{
						EffectManager::Get().StopEffectRoot(bubbleHandle);
					}
				}
				activePoisonClouds_.pop_front();
			}

			PoisonCloud cloud;
			cloud.position      = position;
			cloud.remainingTime = PoisonCloud::DURATION;
			cloud.tickTimer     = 0.0f;
			cloud.mushroomOwner = mushroom;

			if (EffectManager::IsAvailable())
			{
				// 胞子散布エフェクト（spore2：1再生で2回出る）を開始する
				// 煙フィールドは UpdatePoisonClouds で spore2 終了後に開始する
				Vector3 sporePos = position;
				sporePos.y += PoisonCloud::EFFECT_Y_OFFSET;
				cloud.sporeHandle = EffectManager::Get().PlayEffect(
					enEffectKind_MushroomPoisonSpore2,
					sporePos,
					Quaternion::Identity,
					Vector3::One
				);
			}

			activePoisonClouds_.push_back(cloud);
		}


		void BattleManager::UpdatePoisonClouds()
		{
			if (!battleCharacter_) return;

			float dt = g_gameTime->GetFrameDeltaTime();

			for (auto it = activePoisonClouds_.begin(); it != activePoisonClouds_.end(); )
			{
				// spore2 の1回目パーティクル消失後に smoke2 を開始する
				// SMOKE_START_DELAY 秒経過したら smoke2 を再生（spore2 の2回目は引き続き再生中）
				if (!it->smokeStarted)
				{
					it->sporeElapsedTime += dt;
					if (it->sporeElapsedTime >= PoisonCloud::SMOKE_START_DELAY)
					{
						it->smokeStarted  = true;
						it->remainingTime = PoisonCloud::DURATION;

						if (EffectManager::IsAvailable())
						{
							Vector3 smokePos = it->position;
							smokePos.y += 1.0f;
							it->smokeHandle = EffectManager::Get().PlayEffect(
								enEffectKind_MushroomPoisonSmoke2,
								smokePos,
								Quaternion::Identity,
								Vector3::One
							);
						}
					}
				}

				// spore2 の再生が完全に終わってからキノコの移動ロックを解除する
				// smoke2 開始後も spore2 の2回目が続くため、IsEffectPlaying で終了を検知する
				if (it->smokeStarted && it->mushroomOwner)
				{
					const bool sporeFinished = (it->sporeHandle == INVALID_EFFECT_HANDLE)
						|| !EffectManager::IsAvailable()
						|| !EffectManager::Get().IsEffectPlaying(it->sporeHandle);
					if (sporeFinished)
					{
						it->mushroomOwner->GetStateMachine()->SetPoisonSporeActive(false);
						it->mushroomOwner = nullptr;
					}
				}

				// 煙が始まるまではカウントダウンしない
				if (!it->smokeStarted)
				{
					++it;
					continue;
				}

				// 毒フィールド展開中、一定間隔でフィールド中心付近に毒泡を発生させる
				// フィールド終了間際（残り時間が BUBBLE_STOP_MARGIN 以下）は新規発生を止め、
				// 既存の泡がフィールド消失前に再生し終わるようにする
				it->bubbleTimer += dt;
				if (it->bubbleTimer >= PoisonCloud::BUBBLE_INTERVAL
					&& it->remainingTime > PoisonCloud::BUBBLE_STOP_MARGIN
					&& EffectManager::IsAvailable())
				{
					it->bubbleTimer -= PoisonCloud::BUBBLE_INTERVAL;

					const float angle  = (static_cast<float>(rand()) / RAND_MAX) * (3.14159265f * 2.f);
					const float radius = (static_cast<float>(rand()) / RAND_MAX) * PoisonCloud::BUBBLE_SPAWN_RADIUS;
					Vector3 bubblePos = it->position;
					bubblePos.x += cosf(angle) * radius;
					bubblePos.z += sinf(angle) * radius;

					// poison_bubble1 / poison_bubble2 をランダムに選んで再生する
					const int bubbleKind = (rand() % 2 == 0)
						? enEffectKind_MushroomPoisonBubble1
						: enEffectKind_MushroomPoisonBubble2;

					EffectHandle bubbleHandle = EffectManager::Get().PlayEffect(
						bubbleKind,
						bubblePos,
						Quaternion::Identity,
						Vector3::One
					);
					it->bubbleHandles.push_back(bubbleHandle);
				}

				it->remainingTime -= dt;

				if (it->remainingTime <= 0.0f)
				{
					// 期限切れ → 持続煙エフェクトのルートを停止（既存パーティクルはエフェクシア側のフェードアウトで自然消滅させる）
					if (EffectManager::IsAvailable() && it->smokeHandle != INVALID_EFFECT_HANDLE)
					{
						EffectManager::Get().StopEffectRoot(it->smokeHandle);
					}
					// フィールド終了時、まだ再生中の毒泡も一緒に停止する（フィールド消失後に泡だけ残るのを防ぐ）
					if (EffectManager::IsAvailable())
					{
						for (EffectHandle bubbleHandle : it->bubbleHandles)
						{
							EffectManager::Get().StopEffectRoot(bubbleHandle);
						}
					}
					it = activePoisonClouds_.erase(it);
					continue;
				}

				it->tickTimer += dt;
				if (it->tickTimer >= PoisonCloud::TICK_INTERVAL)
				{
					it->tickTimer -= PoisonCloud::TICK_INTERVAL;

					// プレイヤーが雲の範囲内にいるかチェック（Y方向は無視）
					Vector3 diff = battleCharacter_->transform.position - it->position;
					diff.y = 0.0f;
					if (diff.Length() <= PoisonCloud::RADIUS && !tutorialNoDamage_)
					{
						// 毎秒 最大HP×3% のダメージ（ノックバック・無敵なし）
						float maxHp = battleCharacter_->GetStatus()->GetMaxHp();
						float damage = maxHp * PoisonCloud::DAMAGE_PER_TICK_RATE;
						float newHp = battleCharacter_->GetStatus()->GetCurrentHp() - damage;
						newHp = max(newHp, 0.0f);
						battleCharacter_->GetStatus()->SetCurrentHp(newHp);

						if (newHp <= 0.0f)
						{
							battleCharacter_->GetStateMachine()->OnDead();
						}
					}
				}

				++it;
			}
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

			/** クリティカル判定 */
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

			/**
             * 松明ライト（壁沿い 8本)
             * TBDR が無効のときはスキップ（スポーンライトは常に動く）
             */
			if (torchLightsEnabled_ && g_renderingEngine->IsTBDREnabled() && battleCharacter_ != nullptr)
			{
				const float HEIGHT = battleCharacter_->transform.position.y + 80.0f;
				constexpr float RADIUS = 650.f;
				constexpr float RANGE  = 200.f;
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

			/** スポーンライト（動的なフェードアウト） */
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
			if (app::effect::SwordDecalManager::IsAvailable())
			{
				app::effect::SwordDecalManager::Get().Render(rc);
			}

#ifdef K2_DEBUG
			// デバッグ用：Stoneとびかかり攻撃が視野角外で本当に発動していないかを画面上で確認する
			{
				if (!stonePounceDebugFont_)
				{
					stonePounceDebugFont_ = std::make_unique<Font>();
					stonePounceDebugFontShadow_ = std::make_unique<Font>();
				}

				wchar_t text[128];
				swprintf(text, L"Pounce Triggered:%d  BlockedByCamera:%d",
					app::actor::StonePounceDebugStats::triggeredCount,
					app::actor::StonePounceDebugStats::blockedByCameraCount);

				// 画面左下に表示（pivot(0,0)=左下基準なのでテキストは右上方向へ伸びる）
				const Vector2 pos = { UI_SPACE_WIDTH * -0.48f, UI_SPACE_HEIGHT * -0.48f };

				stonePounceDebugFont_->Begin(rc);
				stonePounceDebugFontShadow_->Draw(text, { pos.x + 3.0f, pos.y - 3.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, 0.0f, 1.0f, { 0.0f, 0.0f });
				stonePounceDebugFont_->Draw(text, pos, { 1.0f, 0.6f, 0.2f, 1.0f }, 0.0f, 1.0f, { 0.0f, 0.0f });
				stonePounceDebugFont_->End(rc);
			}
#endif
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

			/** 敵をポーズ（プレイヤーのDeadアニメは止めない） */
			for (auto* stone : stoneEventCharacters_)
				if (stone) stone->SetPause(v);
			for (auto* mushroom : mushroomEventCharacters_)
				if (mushroom) mushroom->SetPause(v);
			if (eventCharacterSpawnManagerObject_)
				eventCharacterSpawnManagerObject_->SetPause(v);

			/** タイマーを止める */
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
			/** バトル共通パラメーター読み込み */
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterBattleParameter>(MASTER_BATTLE_PARAM_PATH, [](const nlohmann::json& json, app::core::MasterBattleParameter& p)
				{
					p.battleTime = json["battleTime"].get<float>();
				});
			/** ステージ共通パラメーター読み込み */
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
			/** バトルキャラクターパラメーター読み込み */
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
			/** 武器パラメーター読み込み */
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterWeaponParameter>(MASTER_WEAPON_PARAM_PATH,[](const nlohmann::json& json, app::core::MasterWeaponParameter& p)
				{
					p.attackPower = json["attackPower"].get<float>();
				});
			/** イベントキャラクターパラメーター読み込み */
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterEventCharacterParameter>(MASTER_EVENT_CHARACTER_PARAM_PATH, [](const nlohmann::json& json, app::core::MasterEventCharacterParameter& p)
				{
					p.moveSpeed = json["moveSpeed"].get<float>();
					p.jumpMoveSpeed = json["jumpMoveSpeed"].get<float>();
					p.jumpPower = json["jumpPower"].get<float>();
					p.radius = json["radius"].get<float>();
					p.height = json["height"].get<float>();
				});
			/** ストーンイベントキャラクターパラメータ読み込み */
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterStoneEventCharacterParameter>(MASTER_STONE_EVENT_CHARACTER_PARAM_PATH, [](const nlohmann::json& json, app::core::MasterStoneEventCharacterParameter& p)
				{
					p.moveSpeed = json["moveSpeed"].get<float>();
					p.jumpMoveSpeed = json["jumpMoveSpeed"].get<float>();
					p.jumpPower = json["jumpPower"].get<float>();
					p.radius = json["radius"].get<float>();
					p.height = json["height"].get<float>();
					p.hp = json["hp"].get<float>();
					p.tutorialHp = json.value("tutorialHp", p.hp);
					p.tutorialAttackPower = json.value("tutorialAttackPower", 0.0f);
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
			/** マッシュルイベントキャラクターパラメータ読み込み */
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterMushroomEventCharacterParameter>(MASTER_MUSHROOM_EVENT_CHARACTER_PARAM_PATH, [](const nlohmann::json& json, app::core::MasterMushroomEventCharacterParameter& p)
				{
					p.moveSpeed = json["moveSpeed"].get<float>();
					p.jumpMoveSpeed = json["jumpMoveSpeed"].get<float>();
					p.jumpPower = json["jumpPower"].get<float>();
					p.radius = json["radius"].get<float>();
					p.height = json["height"].get<float>();
					p.hp = json["hp"].get<float>();
					p.tutorialHp = json.value("tutorialHp", p.hp);
					p.tutorialAttackPower = json.value("tutorialAttackPower", 0.0f);
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
