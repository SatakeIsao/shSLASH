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
		/**
         * ゲーム内で外部から状態をセットして使うタイプのカメラコントローラー
         */
        class GameCamera : public ICameraController
        {
            appCameraController(GameCamera);


        private:
            CameraData data_;


        public:
            /**
             * 外部から状態をセットする
             * NOTE: BattleManagerなどが呼ぶ
             */
            void SetState(const CameraData& data)
            {
                data_ = data;
            }

            void Update() override
            {
                // 基本何もしない。
                // 必要ならここで画面揺れ(Shake)のオフセットを加算する処理などを入れても良い
            }

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