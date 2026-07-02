/**
 * TitleMenuManager.h
 */
#pragma once

namespace app {
    namespace ui {
        class Layout;
    }

    namespace title {
        enum class TitleMenuState {
            enClosed,
            enPressAnyButton,
            enMainMenu,
            enOptionMenu,
        };

        class TitleMenuManager {
        private:
            app::ui::Layout* pressAnyButtonLayout_ = nullptr;
            app::ui::Layout* mainMenuLayout_ = nullptr; 
            app::ui::Layout* optionMenuLayout_ = nullptr;

            TitleMenuState currentState_ = TitleMenuState::enPressAnyButton;
            TitleMenuState nextState_ = TitleMenuState::enClosed;
            bool isPlayingAnimation_ = false;

            bool isGameStartDecided_ = false;
            bool isTutorialDecided_ = false;

        private:
            TitleMenuManager();
            ~TitleMenuManager();

        public:
            void Update();
            void Render(RenderContext& rc);

            void ChangeState(TitleMenuState nextState) {
                nextState_ = nextState;
                isPlayingAnimation_ = true;
            }

        private:
            void CreateMenu();

        public:
            bool IsGameStartDecided() const { return isGameStartDecided_; }
            bool IsTutorialDecided() const { return isTutorialDecided_; }

            static void Initialize() {
                if (instance_ == nullptr) instance_ = new TitleMenuManager();
            }
            static TitleMenuManager& Get() { return *instance_; }
            static void Finalize() {
                if (instance_ != nullptr) {
                    delete instance_;
                    instance_ = nullptr;
                }
            }

        private:
            static TitleMenuManager* instance_;
        };
    }
}