/**
 * Curve.h
 * 線形保管等のCurve処理群
 */
#pragma once


namespace app
{
	namespace util
	{

        /**
         * イージングの種類
         * NOTE: 線形補間、イーズイン、イーズアウト、イーズインアウト
         */
        enum class EasingType { Linear, EaseIn, EaseOut, EaseInOut };
        /**
         * ループの種類
         * NOTE: 片道、周回(上から下、上から下を繰り返す)、往復(上から下、下から上を繰り返す)
         */
        enum class LoopMode { Once, Loop, PingPong };




		/**
         * 汎用的なカーブクラステンプレート
         */
        template <typename T>
        class Curve
        {
        private:
            /** 始める数値 */
            T startValue_;
            /** 終わる数値 */
            T endValue_;
            /** 時間の間隔 */
            float duration_;
            /** 現在の時間 */
            float currentTime_;
            /** イージングタイプ */
            EasingType easingType_;
            /** ループモード */
            LoopMode loopMode_;
            /** 再生するか */
            bool isPlaying_;
            /** 方向 */
            int direction_;


        public:
            Curve()
                : currentTime_(0)
                , duration_(1.0f)
                , isPlaying_(false)
                , direction_(1)
            {
            }

            /* 初期化 */
            void Initialize(const T& start, const T& end, const float timeSec, const EasingType type = EasingType::EaseInOut, const LoopMode loopMode = LoopMode::Once)
            {
                startValue_ = start;
                endValue_ = end;
                duration_ = max(0.0001f, timeSec);
                easingType_ = type;
                loopMode_ = loopMode;
                currentTime_ = 0.0f;
                direction_ = 1;
            }

            /** 再生 */
            void Play()
            {
                isPlaying_ = true;
            }

            /** 停止 */
            void Stop()
            {
                isPlaying_ = false;
			}

            /** 更新 */
            void Update(float deltaTime)
            {
				// 再生していなければ何もしない
                if (!isPlaying_) return;

				// 時間を進める
                currentTime_ += loopMode_ == LoopMode::PingPong ? deltaTime * direction_ : deltaTime;

				// 終了判定とループ処理
                if (currentTime_ >= duration_) {
                    if (loopMode_ == LoopMode::Once) {
                        currentTime_ = duration_;
                        isPlaying_ = false;
                    }
                    else if (loopMode_ == LoopMode::Loop) {
                        currentTime_ = 0.0f;
                    }
                    else if (loopMode_ == LoopMode::PingPong) {
                        currentTime_ = duration_;
                        direction_ = -1;
                    }
                }
                else if (currentTime_ <= 0.0f) {
                    if (loopMode_ == LoopMode::PingPong) {
                        currentTime_ = 0.0f;
                        direction_ = 1;
                    }
                }
            }

			/** 現在の値を取得 */
            T GetCurrentValue() const
            {
                float t = clamp<float>(currentTime_ / duration_, 0.0f, 1.0f);
                float rate = ApplyEasingInternal(t);
                return startValue_ + (endValue_ - startValue_) * rate;
            }

			/** 再生中か取得 */
            bool IsPlaying() const { return isPlaying_; }


        private:
			/** イージング適用 */
            float ApplyEasingInternal(float t) const
            {
                switch (easingType_) {
                case EasingType::Linear:    return t;
                case EasingType::EaseIn:    return t * t;
                case EasingType::EaseOut:   return t * (2.0f - t);
                case EasingType::EaseInOut:
                    if (t < 0.5f) return 2.0f * t * t;
                    else          return -1.0f + (4.0f - 2.0f * t) * t;
                }
                return t;
            }
        };


        using FloatCurve = Curve<float>;
        using Vector2Curve = Curve<Vector2>;
        using Vector3Curve = Curve<Vector3>;
        using Vector4Curve = Curve<Vector4>;
	}
}