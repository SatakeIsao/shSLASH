/**
 * Types.h
 * サウンド用の定数など必要な情報を定義するファイル
 */
#pragma once
#include <string>

namespace app
{

	/** サウンドの種類 */
	enum class SoundKind : uint32_t
	{
		SE = 0,
		Button = SE,
		ButtonDecision,
		ButtonReturn,
		ButtonMove,
		ButtonPause,
		ButtonReset,
		TitleSlash,
		TitleBreak,
		AtkMushroom,
		AtkStone,
		AtkCharge,
		AtkWeak,
		Avoidance,
		Charging,
		DamagePlayer,
		DeadPlayer,
		DeadMushroom,
		DeadStone,
		Defence,
		DefenceKnockback,
		FootStep,
		InjuredFootStep,
		GaugeUp,
		HpDanger,
		HpHealth,
		KipUp,
		KnockbackMushroom,
		KnockbackStone,
		LevelUp,
		Slide,
		TaskClear,
		SEMax,
		BGM = SEMax,
		Game = BGM,
		Title,
		BGMMax,
		//Title,
		Voice = BGMMax,
		Startup00 = Voice,
		Startup01,
		Damage,
		CountdownVoice5,
		CountdownVoice4,
		CountdownVoice3,
		CountdownVoice2,
		CountdownVoice1,
		CountdownVoiceStart,
		CountdownVoiceTimeUp,
		GameOverVoice,
		VoiceMax,
		Max = VoiceMax,
		None = Max,
	};


	/** サウンドの情報の構造体 */
	struct SoundInformation
	{
		std::string assetPath;
		//
		SoundInformation(const std::string& path) : assetPath(path) {}
	};


	/** 情報を保持 */
	static SoundInformation soundInformation[static_cast<uint32_t>(SoundKind::Max)] =
	{
		// SE
		SoundInformation("Assets/sound/se/button.wav"),
		SoundInformation("Assets/sound/se/buttonDecision.wav"),
		SoundInformation("Assets/sound/se/buttonReturn.wav"),
		SoundInformation("Assets/sound/se/buttonMove.wav"),
		SoundInformation("Assets/sound/se/buttonPause.wav"),
		SoundInformation("Assets/sound/se/buttonReset.wav"),
		SoundInformation("Assets/sound/se/titleSlash.wav"),
		SoundInformation("Assets/sound/se/titleBreak.wav"),
		SoundInformation("Assets/sound/se/atkMushroom.wav"),
		SoundInformation("Assets/sound/se/atkStone.wav"),
		SoundInformation("Assets/sound/se/atkCharge.wav"),
		SoundInformation("Assets/sound/se/atkWeak.wav"),
		SoundInformation("Assets/sound/se/avoidance.wav"),
		SoundInformation("Assets/sound/se/charging.wav"),
		SoundInformation("Assets/sound/se/damagePlayer.wav"),
		SoundInformation("Assets/sound/se/deadPlayer.wav"),
		SoundInformation("Assets/sound/se/deadMushroom.wav"),
		SoundInformation("Assets/sound/se/deadStone.wav"),
		SoundInformation("Assets/sound/se/defence.wav"),
		SoundInformation("Assets/sound/se/defenceKnockback.wav"),
		SoundInformation("Assets/sound/se/footStep.wav"),
		SoundInformation("Assets/sound/se/InjuredFootStep.wav"),
		SoundInformation("Assets/sound/se/gaugeUp.wav"),
		SoundInformation("Assets/sound/se/hpDanger.wav"),
		SoundInformation("Assets/sound/se/hpHealth.wav"),
		SoundInformation("Assets/sound/se/kipUp.wav"),
		SoundInformation("Assets/sound/se/knockbackMushroom.wav"),
		SoundInformation("Assets/sound/se/knockbackStone.wav"),
		SoundInformation("Assets/sound/se/levelUp.wav"),
		SoundInformation("Assets/sound/se/slide.wav"),
		SoundInformation("Assets/sound/se/taskClear.wav"),
		// BGM
		SoundInformation("Assets/sound/bgm/battle.wav"),
		SoundInformation("Assets/sound/bgm/title.wav"),
		// Voice
		SoundInformation("Assets/sound/voice/StartupVoice_00.wav"),
		SoundInformation("Assets/sound/voice/StartupVoice_01.wav"),
		SoundInformation("Assets/sound/voice/damageVoice.wav"),
		SoundInformation("Assets/sound/voice/5.wav"),
		SoundInformation("Assets/sound/voice/4.wav"),
		SoundInformation("Assets/sound/voice/3.wav"),
		SoundInformation("Assets/sound/voice/2.wav"),
		SoundInformation("Assets/sound/voice/1.wav"),
		SoundInformation("Assets/sound/voice/start.wav"),
		SoundInformation("Assets/sound/voice/timeup.wav"),
		SoundInformation("Assets/sound/voice/gameover.wav"),
	};


}