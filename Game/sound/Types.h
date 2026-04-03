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
		SEMax,
		BGM = SEMax,
		Game = BGM,
		BGMMax,
		//Title,
		Voice = BGMMax,
		Startup00 = Voice,
		Startup01,
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
		// BGM
		SoundInformation("Assets/sound/bgm/explore_bgm.wav"),
		// Voice
		SoundInformation("Assets/sound/voice/StartupVoice_00.wav"),
		SoundInformation("Assets/sound/voice/StartupVoice_01.wav"),
	};


}