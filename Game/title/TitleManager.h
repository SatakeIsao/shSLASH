/**
 * TitleManager.h
 */
#pragma once 

namespace app
{
    namespace core
    {
        class TitleMenuManagerObject;
    }

    namespace title
    {
        class TitleManager : public Noncopyable
        {
        private:
            app::core::TitleMenuManagerObject* titleMenuManagerObject_ = nullptr;

            TitleManager();
            ~TitleManager();

        public:
            void Start();
            void Update();

            bool IsGameStartDecided() const;
            bool IsTutorialDecided() const;

        public:
            static void Initialize();
            static TitleManager& Get();
            static void Finalize();

        private:
            static TitleManager* instance_;
        };
    }
}