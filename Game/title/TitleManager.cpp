/**
 * TitleManager.cpp
 */
#include "stdafx.h"
#include "TitleManager.h"
#include "core/TitleMenuManagerObject.h"
#include "core/TitleMenuManager.h" 

namespace app
{
    namespace title
    {
        TitleManager* TitleManager::instance_ = nullptr;

        TitleManager::TitleManager() {}

        TitleManager::~TitleManager()
        {
            if (titleMenuManagerObject_) {
                DeleteGO(titleMenuManagerObject_);
            }
        }

        bool TitleManager::IsGameStartDecided() const
        {
            return app::core::TitleMenuManager::Get().IsGameStartDecided();
        }

        bool TitleManager::IsTutorialDecided() const
        {
            return app::core::TitleMenuManager::Get().IsTutorialDecided();
        }

        void TitleManager::Start()
        {
            titleMenuManagerObject_ = NewGO<app::core::TitleMenuManagerObject>(static_cast<uint8_t>(ObjectPriority::Default));
        }

        void TitleManager::Update() {}

        void TitleManager::Initialize() {
            if (instance_ == nullptr) instance_ = new TitleManager();
        }
        TitleManager& TitleManager::Get() {
            return *instance_;
        }
        void TitleManager::Finalize() {
            if (instance_ != nullptr) {
                delete instance_;
                instance_ = nullptr;
            }
        }
    }
}