/**
 * EffectManager.h
 * エフェクト管理
 * 必要なエフェクトファイルを読み込んだり再生したりなど管理する
 */
#pragma once
#include "effect/Types.h"


 /** エフェクト用のハンドル名 */
using EffectHandle = uint32_t;
/** ハンドル無効値 */
static constexpr EffectHandle INVALID_EFFECT_HANDLE = 0xffffffff;


/**
 * エフェクト管理クラス
 */
class EffectManager
{
private:
    /**
     * エフェクトインスタンスを保持
     * NOTE: ループだけ管理
     *       ループではない場合、K2Engine側で削除処理をしているため
     */
    std::map<EffectHandle, EffectEmitter*> m_effectList;
    /**
     * マップで参照するようにハンドル数を保持
     * NOTE: 各エフェクトソースを参照するための数値はユニークな数値になる
     */
    EffectHandle m_effectHandleCount = 0;


private:
    EffectManager();
    ~EffectManager();


public:
    /**
     * 更新処理
     * NOTE: 不要になったインスタンスの削除など
     */
    void Update();


public:
    /** エフェクト再生 */
    EffectHandle PlayEffect(const int kind, const Vector3& position, const Quaternion& rotation, const Vector3& scale);
    /** エフェクト停止 */
    void StopEffect(const EffectHandle handle);


    EffectEmitter* FindEffect(const EffectHandle handle)
    {
        auto it = m_effectList.find(handle);
        if (it != m_effectList.end()) {
            return it->second;
        }
        K2_ASSERT(false, "削除済みか追加されていないエフェクトにアクセスしようとしています。\n");
        return nullptr;
    }




    /**
     * シングルトン用
     */
public:
    /**
     * インスタンスを作る
     */
    static void Initialize()
    {
        if (m_instance == nullptr)
        {
            m_instance = new EffectManager();
        }
    }


    /**
     * インスタンスを取得
     */
    static EffectManager& Get()
    {
        return *m_instance;
    }


    /**
     * インスタンスを破棄
     */
    static void Finalize()
    {
        if (m_instance != nullptr)
        {
            delete m_instance;
            m_instance = nullptr;
        }
    }

private:
    /** シングルトンインスタンス */
    static EffectManager* m_instance;
};




/***********************************************/


/**
 * エフェクトマネージャーを管理するクラス
 * 
 * エフェクトの読み込みのため
 * mainでエフェクトの読み込みをした場合、エフェクシアの初期化が終わってないためうまく動作しない
 * そのため、エフェクシアの初期化が終わる次のフレームでエフェクトマネージャーの生成をしたい
 * 0フレーム = Main + エフェクシアの初期化 + エフェクトマネージャーオブジェクトの生成
 * 1フレーム = エフェクトマネージャーオブジェクトのスタート関数(エフェクトデータの読み込み)
 */
class EffectManagerObject : public IGameObject
{
public:
    EffectManagerObject();
    ~EffectManagerObject();

    bool Start() override;
    void Update() override;
    void Render(RenderContext& rc);

    /** エフェクト再生をEffectManagerに委譲 */
    EffectHandle PlayEffect(const int kind, const Vector3& position, const Quaternion& rotation, const Vector3& scale);
    /** エフェクト停止をEffectMangerに委譲 */
    void StopEffect(const EffectHandle handle);
};