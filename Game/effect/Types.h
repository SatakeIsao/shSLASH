/**
 * Types.h
 * エフェクト用の定数など必要な情報を定義するファイル
 */
#pragma once
#include <string>


/** エフェクトの種類 */
enum enEffectKind
{
	enEffectKind= 0,
	enEffectKind_TestPlayerKnockBack = enEffectKind,
	enEffectKind_SlimeKnockBack,
	enEffectKind_PlayerAvoidance,
	enEffectKind_PlayerAvoidanceDust,
	enEffectKind_PlayerAvoidanceJust,
	enEffectKind_PlayerAttack,
	enEffectKind_PlayerAttackCharge_Start,
	enEffectKind_PlayerAttackCharge_Slash,
	enEffectKind_PlayerAttackCharge_End,
	enEffectKind_PlayerKnockBack,
	enEffectKind_PlayerLevelUp,
	enEffectKind_PlayerSpawn,
	enEffectKind_PlayerUIHealHP,
	enEffectKind_SlimeAttack,
	enEffectKind_StoneAttack,
	enEffectKind_StoneDead,
	enEffectKind_StoneKnockBack,
	enEffectKind_StonePredictionAtk,
	enEffectKind_StoneSpawn,
	enEffectKind_MushroomAttack,
	enEffectKind_MushroomDead,
	enEffectKind_MushroomKnockBack,
	enEffectKind_MushroomPredictionAtk,
	enEffectKind_MushroomSpawn,
	enEffectKind_PlayerChargeLevel1,
	enEffectKind_PlayerChargeLevel2,
	enEffectKind_PlayerChargeLevel3,
	enEffectKind_Max,
	enEffectKind_None = enEffectKind_Max,
};


/** エフェクトの情報の構造体 */
struct EffectInformation
{
	const char16_t* assetPath;
	//
	EffectInformation(const char16_t* path) : assetPath(path) {}
};


/** 情報を保持 */
static EffectInformation effectInformation[enEffectKind_Max] =
{
	EffectInformation(u"Assets/effect/testPlayer_knockback.efk"),
	EffectInformation(u"Assets/effect/slimeKnockback.efk"),
	EffectInformation(u"Assets/effect/playerAvoidance.efk"),
	EffectInformation(u"Assets/effect/playerAvoidanceDust.efk"),
	EffectInformation(u"Assets/effect/playerAvoidanceJust.efk"),
	EffectInformation(u"Assets/effect/playerDefaultSlash.efk"),
	EffectInformation(u"Assets/effect/playerChargeAtk_Start.efk"),
	EffectInformation(u"Assets/effect/playerChargeSlash.efk"),
	EffectInformation(u"Assets/effect/playerChargeAtk_End.efk"),
	EffectInformation(u"Assets/effect/playerKnockback.efk"),
	EffectInformation(u"Assets/effect/playerLevelUp.efk"),
	EffectInformation(u"Assets/effect/playerSpawn.efk"),
	EffectInformation(u"Assets/effect/playerUIHealHP.efk"),
	EffectInformation(u"Assets/effect/slimeAtk.efk"),
	EffectInformation(u"Assets/effect/slime_attack.efk"),
	EffectInformation(u"Assets/effect/stoneDead.efk"),
	EffectInformation(u"Assets/effect/stoneKnockBack.efk"),
	EffectInformation(u"Assets/effect/stonePredictionAtk.efk"),
	EffectInformation(u"Assets/effect/stoneSpawn.efk"),
	EffectInformation(u"Assets/effect/mushroomAttack.efk"),
	EffectInformation(u"Assets/effect/mushroomDead.efk"),
	EffectInformation(u"Assets/effect/mushroomKnockBack.efk"),
	EffectInformation(u"Assets/effect/mushroomPredictionAtk.efk"),
	EffectInformation(u"Assets/effect/mushroomSpawn.efk"),
	EffectInformation(u"Assets/effect/playerChargeAtk_Level1.efk"),
	EffectInformation(u"Assets/effect/playerChargeAtk_Level2.efk"),
	EffectInformation(u"Assets/effect/playerChargeAtk_Level3.efk"),

};

