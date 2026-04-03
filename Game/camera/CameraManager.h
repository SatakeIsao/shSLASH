/**
 * CameraManager.h
 * カメラ管理
 */
#pragma once
#include "CameraCommon.h"
#include "SpringCamera.h"


namespace app
{
	namespace camera
	{
        class CameraManager
        {
        private:
            std::map<uint32_t, std::shared_ptr<ICameraController>> controllers_;
            std::shared_ptr<ICameraController> current_;
            std::shared_ptr<ICameraController> next_;
#if defined(APP_DEBUG)
            std::shared_ptr<ICameraController> prev_;
#endif

            std::unique_ptr<SpringCamera> springCamera_;

            /** ブレンド用 */
            bool isBlending_ = false;
            float blendDuration_ = 0.0f;
            float blendTimer_ = 0.0f;
            CameraData blendStartData_;

            /** 描画で使用されるカメラ */
            nsK2EngineLow::Camera* engineCamera_ = nullptr;


        private:
            CameraManager();
            ~CameraManager();


        public:
            /**
             * 初期化
             * エンジン側の実体カメラを設定
             */
            void Setup(nsK2EngineLow::Camera* engineCamera);

            /**
             * 更新処理
             * ブレンド計算とエンジンカメラへの反映
             */
            void Update(const float deltaTime);

            /** コントローラーの登録・解除 */
            void Register(const uint32_t nameHash, RefCameraController controller);
            void Unregister(const uint32_t nameHash);

            /**
             * カメラ切り替え
             * NOTE: blendTime秒かけて遷移し0なら即時。
             */
            void SwitchCamera(const uint32_t nameHash, const float blendTime = 0.0f);
            void SwitchCamera(RefCameraController controller, const float blendTime = 0.0f);
#if defined(APP_DEBUG)
            /** デバッグ用: 前のカメラに戻す */
            void SwitchPrevCamera(const float blendTime = 0.0f)
            {
                if (prev_) {
                    SwitchCamera(prev_, blendTime);
                }
            }
#endif // APP_DEBUG

			/** 現在のカメラデータを取得 */
            const CameraData& GetCurrentCameraData() const
            {
                if (current_) {
                    return current_->GetCameraData();
                }
                static CameraData defaultData;
                return defaultData;
			}




            /**
             * シングルトン関連
             */
        private:
			static CameraManager* instance_;


        public:
            static void Initialize()
            {
                if (instance_ == nullptr) {
                    instance_ = new CameraManager();
                }
			}
            static CameraManager& Get() { return *instance_; }
            static void Finalize()
            {
                if (instance_ != nullptr) {
                    delete instance_;
                    instance_ = nullptr;
                }
			}
        };
	}
}