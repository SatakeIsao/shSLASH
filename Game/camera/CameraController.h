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

            float shakeTimer_     = 0.0f;
            float shakeDuration_  = 0.0f;
            float shakeIntensity_ = 0.0f;
            float shakeElapsed_   = 0.0f;
            bool  isUpwardShake_  = false;
            bool  isShakeEnabled_ = true;


        public:
            /**
             * 外部から状態をセットする
             * NOTE: BattleManagerなどが呼ぶ
             */
            void SetState(const CameraData& data)
            {
                data_ = data;
            }

            /** 通常のカメラシェイク（横＋縦の揺れ） */
            void StartShake(ShakeSize size);
            /** 下から上への掃き上げシェイク（2コンボ目用） */
            void StartShakeUpward(ShakeSize size);

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