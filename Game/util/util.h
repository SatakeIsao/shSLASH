/**
 * Utilファイル
 */
#pragma once
#include <chrono>


namespace app
{
    /** 汎用的なclamp関数テンプレート */
    template <typename T>
    T clamp(T value, T low, T high) {
        if (value < low) {
            return low;
        }
        if (value > high) {
            return high;
        }
        return value;
    }




    /**
     * 目標時間に対する経過率を計算するクラス
     */
    class ComputeRate
    {
    private:
	    float m_elapsedTime = 0.0f;
	    float m_targetTime = 1.0f;
	    bool m_isLoop = false;


    public:
        ComputeRate() {}
        ~ComputeRate() {}

        /** 初期化 */
        void Initialize(const float targetTime, const bool isLoop = false)
        {
            m_targetTime = targetTime;
		    m_elapsedTime = 0.0f;
		    m_isLoop = isLoop;
        }

        /** 更新し経過率を返す */
        float Update()
        {
            const float deltaTime = g_gameTime->GetFrameDeltaTime();
		    m_elapsedTime += deltaTime;
            if(m_elapsedTime > m_targetTime) {
			    m_elapsedTime = m_targetTime;
                if (m_isLoop) {
                    m_elapsedTime = 0.0f;
                }
		    }
		    const float rate = m_elapsedTime / m_targetTime;
            return rate;
        }
    };




    /**
     * 処理時間計測用スコープタイマー
     */
    class ScopeTimer
    {
    private:
        const char* m_name;
        std::chrono::steady_clock::time_point m_start;


    public:
        ScopeTimer(const char* name)
            : m_name(name)
            , m_start(std::chrono::steady_clock::now()) {
        }

        ~ScopeTimer() {
            // ミリ秒単位で時間取得
            auto end = std::chrono::steady_clock::now();
            auto duration = end - m_start;
            double ms = std::chrono::duration<double, std::milli>(duration).count();
            K2_LOG("ScopeTimer: %s : %fms \n", m_name, ms);
        }
    };
}

#if defined(APP_DEBUG)
    #define appScopeTimer(name) app::ScopeTimer timer##__LINE__(name);
#else
    #define appScopeTimer(name)
#endif
