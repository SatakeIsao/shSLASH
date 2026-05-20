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
	enEffectKind_PlayerAttack,
	enEffectKind_PlayerAttackCharge_Start,
	enEffectKind_PlayerAttackCharge_End,
	enEffectKind_PlayerKnockBack,
	enEffectKind_PlayerLevelUp,
	enEffectKind_SlimeAttack,
	enEffectKind_StoneAttack,
	enEffectKind_StoneDead,
	enEffectKind_StoneKnockBack,
	enEffectKind_StoneSpawn,
	enEffectKind_MushroomAttack,
	enEffectKind_MushroomDead,
	enEffectKind_MushroomKnockBack,
	enEffectKind_MushroomSpawn,
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
	EffectInformation(u"Assets/effect/slime_knockback.efk"),
	EffectInformation(u"Assets/effect/playerAvoidance.efk"),
	EffectInformation(u"Assets/effect/playerAvoidanceDust.efk"),
	EffectInformation(u"Assets/effect/atk2.efk"),
	EffectInformation(u"Assets/effect/playerChargeAtk_Start.efk"),
	EffectInformation(u"Assets/effect/atk.efk"),
	EffectInformation(u"Assets/effect/player_knockback.efk"),
	EffectInformation(u"Assets/effect/playerLevelUp.efk"),
	EffectInformation(u"Assets/effect/slime_attack.efk"),
	EffectInformation(u"Assets/effect/stoneAttack2.efk"),
	EffectInformation(u"Assets/effect/stoneDead.efk"),
	EffectInformation(u"Assets/effect/stoneKnockBack.efk"),
	EffectInformation(u"Assets/effect/stoneSpawn.efk"),
	EffectInformation(u"Assets/effect/mushroomAttack.efk"),
	EffectInformation(u"Assets/effect/mushroomDead.efk"),
	EffectInformation(u"Assets/effect/mushroomKnockBack.efk"),
	EffectInformation(u"Assets/effect/mushroomSpawn.efk"),

};

