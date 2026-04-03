#pragma once
#include "util/CRC32.h"


#define appScene(name)\
public:\
	static constexpr uint32_t ID() { return Hash32(#name); }


/** かぶることはないはずなので最大値を無効値とする */
static constexpr uint32_t INVALID_SCENE_ID = 0xffffffff;


/**
 * 基底クラス
 */
class IScene
{
public:
	IScene() {}
	virtual ~IScene() {} 

	/** 純粋仮想関数:派生クラスで実装されてなかったらエラーが出る */
	virtual bool Start()   = 0; 
	virtual void Update() = 0;
	virtual void Render(RenderContext& rc) = 0;

	/** 
	 * 次のシーンを要求する。
	 * idは次のシーンのIDを返す。
	 * trueならば要求成功、falseならば失敗
	 */
	virtual bool RequestScene(uint32_t& id, float& waitTime) = 0;
};

