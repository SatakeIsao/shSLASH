/**
 * CameraController.h
 * カメラコントローラー群
 */
#pragma once
#include "CameraCommon.h"


namespace app
{
	namespace camera
	{
        /** カメラシェイクの強さ */
        enum class ShakeSize { Small, Medium, Large };


		/**
         * ゲーム内で外部から状態をセットして使うタイプのカメラコントローラー
         */
        class GameCamera : public ICameraController
        {
            appCameraController(GameCamera);


        private:
            CameraData data_;
            float baseFov_ = Math::DegToRad(60.0f);

            float shakeTimer_     = 0.0f;
            float shakeDuration_  = 0.0f;
            float shakeIntensity_ = 0.0f;
            float shakeElapsed_   = 0.0f;
            bool  isUpwardShake_  = false;
            bool  isShakeEnabled_ = true;

            float fovCurrent_     = Math::DegToRad(60.0f); // 現在の実FOV
            float fovFrom_        = Math::DegToRad(60.0f); // 補間開始時のFOV
            float fovTarget_      = Math::DegToRad(60.0f); // 目標FOV
            float fovNarrowTime_  = 0.30f;                 // 狭窄の線形補間時間（秒）
            float fovReturnTime_  = 0.12f;                 // 復帰の線形補間時間（秒）
            float fovNarrowTimer_ = 0.0f;                  // 現在の補間タイマー
            float fovIdleTimer_   = 999.0f;                // 最後の OnCharging/OnAttackHit からの経過秒
            float fovIdleThresh_  = 0.0f;                  // この秒数を超えたら baseFov_ へ復帰
            bool  fovReturning_   = false;                 // 復帰フェーズ中か


        public:
            /**
             * 外部から状態をセットする
             * NOTE: BattleManagerなどが呼ぶ
             */
            void SetState(const CameraData& data);

            /** 通常のカメラシェイク（横＋縦の揺れ） */
            void StartShake(ShakeSize size);
            /** 下から上への掃き上げシェイク（2コンボ目用） */
            void StartShakeUpward(ShakeSize size);

            /**
             * 攻撃ヒット時に呼ぶ。アイドルタイマーをリセットし、狭窄を維持する。
             * 攻撃が止んで idleThreshold 秒経過すると元のFOVに戻る。
             */
            void OnAttackHit(float targetFovDeg, float fadeInDuration, float fadeOutDuration, float idleThreshold);

            /**
             * 溜め中に毎フレーム呼ぶ。chargingLevel 0=開始 / 1/2/3=各段階。
             * 呼ぶのをやめるとアイドルタイマーが走り始めて元のFOVに戻る。
             */
            void OnCharging(int chargingLevel);

            /** 画面揺れの有効・無効を切り替える */
            void SetShakeEnabled(bool enabled) { isShakeEnabled_ = enabled; }
            bool IsShakeEnabled() const { return isShakeEnabled_; }

            void Update() override;

            const CameraData& GetCameraData() const override { return data_; }
        };




#if defined(APP_DEBUG)
		/**
         * デバッグ用カメラコントローラー
         */
        class DebugCamera : public ICameraController
        {
			appCameraController(DebugCamera);


        private:
            CameraData cameraData_;


        public:
            void OnEnter() override;

            void Update() override;

            const CameraData& GetCameraData() const override { return cameraData_; }
        };
#endif // APP_DEBUG
	}
}
