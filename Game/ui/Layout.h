/**
 * Layout.h
 * UIのレイアウト管理
 */
#pragma once
#include "Menu.h"
#include "json/json.hpp"


#ifdef APP_DEBUG
    #define APP_ENABLE_LAYOUT_HOTRELOAD
#endif


namespace app
{
	namespace ui
	{
        class Layout : public Noncopyable
        {
        private:
#ifdef APP_ENABLE_LAYOUT_HOTRELOAD
            time_t lastUpdateTime_ = 0;
#endif // APP_ENABLE_LAYOUT_HOTRELOAD

            std::string filePath_ = "";
            std::unique_ptr<MenuBase> menu_ = nullptr;

        public:
			Layout() {}
			~Layout() {}

            template <typename T>
            void Initialize(const std::string& path)
            {
                filePath_ = path;
                menu_ = std::make_unique<T>();
                Reload();
            }

            MenuBase* GetMenu() const { return menu_.get(); }

            void Update();
            void Render(RenderContext& rc);


            void Reload();
            

        private:
            static UIBase* CreateUI(UICanvas* canvas, const std::string& type, const uint32_t key, const nlohmann::json& item);
        };
	}
}