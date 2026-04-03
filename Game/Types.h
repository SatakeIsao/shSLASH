/**
 * Typesファイル
 */
#pragma once



constexpr uint32_t INVALID_STATE_ID = 0xFFFFFFFF;


/** オブジェクトの優先度 */
enum class ObjectPriority : uint8_t
{
	Default = 0,
	Pause = 200,
	Fade = 250,
};


/** デバッグビルド時のみ有効化 */
#if defined(_DEBUG)
	#define APP_DEBUG
#endif

/** 画像サイズ */
constexpr float MAX_SPRITE_WIDTH = 1920.0f;
constexpr float MAX_SPRITE_HIGHT = 1080.0f;

/** 移動で使用される最小値 */
constexpr float MOVE_MIN_FLOAT = 0.001f;