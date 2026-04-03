/**
 * Menu.h
 * UIパーツ周りから動的な処理を行う基底クラス
 */
#pragma once
#include "UIParts.h"


namespace app
{
	namespace ui
	{
        class UIBase;
        class UICanvas;


        class MenuBase : public Noncopyable
        {
        private:
            std::unique_ptr<UICanvas> canvas_ = nullptr;
            std::unordered_map<uint32_t, UIBase*> uiMap_;

        public:
			MenuBase() {}
			virtual ~MenuBase() { Clear(); }

            virtual void Update()
            {
                if (canvas_) {
                    canvas_->Update();
                }
			}

            virtual void Render(RenderContext& rc)
            {
                if (canvas_) {
                    canvas_->Render(rc);
                }
            }


        public:
            void SetCanvas(UICanvas* canvas) { canvas_.reset(canvas); }
            UICanvas* GetCanvas() { return canvas_.get(); }

            /** UIを登録 */
            void RegisterUI(const uint32_t key, UIBase* ui)
            {
                uiMap_.emplace(key, ui);
            }


			/** UIの登録解除 */
            void UnregisterUI(const uint32_t key)
            {
                uiMap_.erase(key);
			}

            /**
			 * UIの取得
             * NOTE: コード側から特定のボタンやテキストを取得する用
             */
            template <typename T>
            T* GetUI(const uint32_t& key)
            {
                if (uiMap_.count(key)) {
                    return dynamic_cast<T*>(uiMap_[key]);
                }
                return nullptr;
            }

			/** UIが存在するか */
            bool HasUI(const uint32_t& key) const
            {
                return uiMap_.count(key) > 0;
			}

			/** UIのクリア */
            void Clear()
            {
                uiMap_.clear();
            }

            /** 登録されている全UIに対して一括処理を行う */
            void ForEachUI(const std::function<void(UIBase*)>& func)
            {
                for (auto& pair : uiMap_){
                    func(pair.second);
                }
            }

            /**
			 * UIのロジック初期化処理
             * NOTE: ここに「ボタンが押された時の処理」などを書く
             */
            virtual void InitializeLogic()
            {
                //auto* btn = GetUI<UIButton>("StartButton");
                //if (btn) {
                //    // btn->SetDelegate(...) などの処理
                //}
            }
        };
	}
}