/**
 * TutorialScene.h
 * チュートリアルシーン
 */
#pragma once
#include "IScene.h"
#include "ui/Layout.h"
#include "ui/GameOverSequence.h"
#include "ui/TutorialUIObject.h"
#include "ui/TutorialPauseHintObject.h"

namespace app { namespace ui { class TutorialConfirmObject; } }

class TutorialScene : public IScene
{
    appScene(TutorialScene);

private:
    app::ui::GameOverSequence* gameOverSequence_ = nullptr;
    app::ui::TutorialUIObject* tutorialUI_ = nullptr;
    app::ui::TutorialPauseHintObject* tutorialPauseHint_ = nullptr;
    bool isGameOver_ = false;
    bool isLoaded_   = false;
    bool isAllEnemiesDefeated_ = false;
    bool isSkipRequested_ = false;

    std::unique_ptr<app::ui::Layout> confirmLayout_;
    app::ui::TutorialConfirmObject* confirmObject_ = nullptr;
    bool isConfirmOpen_ = false;

public:
    TutorialScene();
    virtual ~TutorialScene();

    virtual bool Start() override;
    virtual void Update() override;
    virtual void Render(RenderContext& rc) override;

    virtual bool RequestScene(uint32_t& id, float& waitTime) override;
    virtual bool IsReadyToFadeIn() const override { return isLoaded_; }
};
