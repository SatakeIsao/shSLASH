/**
 * TitleManager.h
 */
#pragma once 

namespace app
{
    namespace title
    {
        class TitleMenuManagerObject;

        class TitleManager : public Noncopyable
        {
        private:
            app::title::TitleMenuManagerObject* titleMenuManagerObject_ = nullptr;

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