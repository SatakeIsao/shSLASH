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
	enEffectKind_PlayerKnockBack = enEffectKind,
	enEffectKind_SlimeKnockBack,
	enEffectKind_PlayerAttack,
	enEffectKind_SlimeAttack,
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
	EffectInformation(u"Assets/effect/player_knockback.efk"),
	EffectInformation(u"Assets/effect/slime_knockback.efk"),
	EffectInformation(u"Assets/effect/player_attack.efk"),
	EffectInformation(u"Assets/effect/slime_attack.efk"),

};

