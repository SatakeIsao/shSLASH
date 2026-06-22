/**
 * SoundManager.h
 * サウンド管理
 * 必要なサウンドファイルを読み込んだり再生したりなど管理する
 */
#pragma once
#include "sound/Types.h"

namespace app
{

    /** サウンド用のハンドル名 */
    using SoundHandle = uint32_t;
    /** ハンドル無効値 */
    static constexpr SoundHandle INVALID_SOUND_HANDLE = 0xffffffff;
    /** デフォルト音量 */
    static constexpr float VOLUME_DEFAULT_MASTER = 0.50f;
    static constexpr float VOLUME_DEFAULT_BGM = 0.20f;
    static constexpr float VOLUME_DEFAULT_SE = 0.50f;


    /**
     * サウンド管理クラス
     */
    class SoundManager
    {
    private:
        /** BMG用のサウンドソースインスタンスを保持 */
        SoundSource* m_bgm = nullptr;
        /** SE用のサウンドソースインスタンスを保持 */
        std::map<SoundHandle, SoundSource*> m_seList;
        /**
         * マップで参照するようにハンドル数を保持
         * NOTE: 各サウンドソースを参照するための数値はユニークな数値になる
         */
        SoundHandle m_soundHandleCount = 0;


    public:
        enum class SoundVolumeType : uint8_t
        {
            Master,
            BGM,
            SE,
            Max,
        };


    private:
        /** NOTE: union勉強のため書いてみる */
        union
        {
            float volumes[static_cast<uint8_t>(SoundVolumeType::Max)];
            struct
            {
                float volumeMaster_;
                float volumeBGM_;
                float volumeSE_;
            };
        };



    private:
        SoundManager();
        ~SoundManager();


    public:
        /**
         * 更新処理
         * NOTE: 不要になったインスタンスの削除など
         */
        void Update();


    public:
        float GetVolume(const SoundVolumeType volumeType) const
        {
            return volumes[static_cast<uint8_t>(volumeType)];
        }

        void SetVolume(const SoundVolumeType volumeType, const float volume);

        float ComputeOutputVolume(const SoundVolumeType volumeType)
        {
            float masterVolume = GetVolume(SoundVolumeType::Master);
            float volume = masterVolume * volumes[static_cast<uint8_t>(volumeType)];
            return volume;
        }

        /** BGM再生 */
        void PlayBGM(const int kind);
        /** BGM停止 */
        void StopBGM();
        /** BGMのボリューム設定 */
        void SetVolumeBGM(const float volume);

        /** SE再生 */
        SoundHandle PlaySE(const int kind, const bool isLood = false, const bool is3D = false);
        /** SE停止 */
        void StopSE(const SoundHandle handle);
        /** 再生中の全SE停止（シーン遷移時などに使用） */
        void StopAllSE();
        /** SEのボリューム設定 */
        void SetVolumeSE(const SoundHandle handle, float volume);

        SoundSource* FindSE(const SoundHandle handle)
        {
            auto it = m_seList.find(handle);
            if (it != m_seList.end()) {
                return it->second;
            }
            K2_ASSERT(false, "削除済みか追加されていないSEにアクセスしようとしています。\n");
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
                m_instance = new SoundManager();
                // デフォルト音量を初回のみ設定
                m_instance->SetVolume(SoundVolumeType::Master, VOLUME_DEFAULT_MASTER);
                m_instance->SetVolume(SoundVolumeType::BGM, VOLUME_DEFAULT_BGM);
                m_instance->SetVolume(SoundVolumeType::SE, VOLUME_DEFAULT_SE);
            }
        }


        /**
         * インスタンスを取得
         */
        static SoundManager& Get()
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
        static SoundManager* m_instance;
    };

}