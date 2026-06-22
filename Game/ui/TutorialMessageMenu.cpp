#include "stdafx.h"
#include "TutorialMessageMenu.h"
#include "sound/SoundManager.h"
#include "battle/BattleManager.h"
#include "core/PauseManager.h"
#include "json/json.hpp"
#include <fstream>
#include <string>
#include <Windows.h>
#include <sys/stat.h>

namespace
{
    std::wstring Utf8ToWide(const std::string& utf8)
    {
        if (utf8.empty()) return {};
        int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
        if (len <= 0) return {};
        std::wstring result(len, 0);
        MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &result[0], len);
        result.resize(len - 1);  // null終端文字を除去
        return result;
    }
}

namespace app
{
    namespace ui
    {
        void TutorialMessageMenu::InitializeLogic()
        {
            messageWindowIcon_         = GetUI<UIIcon>(Hash32("MessageWindow"));
            messageWindowPracticeIcon_ = GetUI<UIIcon>(Hash32("MessageWindowPractice"));
            if (messageWindowPracticeIcon_) messageWindowPracticeIcon_->isDraw = false;
            titleText_              = GetUI<UIText>(Hash32("MessageWindowTitle"));
            titleLineIcon_          = GetUI<UIIcon>(Hash32("MessageWindowTitleLine"));
            titleLinePracticeIcon_  = GetUI<UIIcon>(Hash32("MessageWindowTitleLinePractice"));
            if (titleLinePracticeIcon_) titleLinePracticeIcon_->isDraw = false;
            messageText_       = GetUI<UIText>(Hash32("MessageWindowText"));
            timerText_         = GetUI<UIText>(Hash32("MessageWindowTimer"));
            nextButtonIcon_    = GetUI<UIIcon>(Hash32("Img_NextButtonA"));
            nextHintText_      = GetUI<UIText>(Hash32("Text_NextHint"));
            clearBannerIcon_   = GetUI<UIIcon>(Hash32("TutorialClearBanner"));
            clearTextIcon_     = GetUI<UIIcon>(Hash32("TutorialClearText"));
            if (clearBannerIcon_) { clearBannerBaseScale_ = clearBannerIcon_->transform.localScale; clearBannerIcon_->isDraw = false; }
            if (clearTextIcon_)   { clearTextBaseScale_   = clearTextIcon_->transform.localScale;   clearTextIcon_->isDraw   = false; }
            clearAnimState_  = ClearAnimState::None;
            finalClearDone_  = false;
            clearPendingAdv_ = false;
            levelExplainIcon_ = GetUI<UIIcon>(Hash32("Img_LevelExplain"));
            phaseExplainIcon_  = GetUI<UIIcon>(Hash32("Img_PhaseExplain"));
            if (levelExplainIcon_) levelExplainIcon_->isDraw = false;
            if (phaseExplainIcon_)  phaseExplainIcon_->isDraw  = false;
            lvPopupPendingAdvance_ = false;
            lvPopupState_          = LvPopupState::None;
            lvPopupCurrentIcon_    = nullptr;
            lvPopupElems_.clear();

            // ホットリロード時にフリーズ状態が残らないように先にリセット
            if (app::battle::BattleManager::IsAvailable())
                app::battle::BattleManager::Get().SetTutorialFreeze(false);

            const int savedIndex = currentIndex_;
            currentIndex_        = 0;
            isAllShown_          = false;
            currentTutorialImages_.clear();
            winAnim_ = WinAnim::None;
            winElems_.clear();
            winOrigScales_.clear();
            slideAnim_      = SlideAnim::None;
            slideElems_.clear();
            slideAnimTimer_ = 0.0f;

            messages_.clear();
            std::ifstream file("Assets/ui/layout/tutorialMessages.json");
            if (file.is_open())
            {
                nlohmann::json j;
                file >> j;

                Vector3 defBodyPos   = Vector3(-125.0f, -280.0f, 0.0f);
                Vector3 defBodyScale = Vector3(0.7f, 0.7f, 1.0f);
                Vector4 defBodyColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

                if (j.contains("layout"))
                {
                    ApplyLayoutOverrides(j["layout"]);

                    if (j["layout"].contains("messageWindowText"))
                    {
                        const auto& n = j["layout"]["messageWindowText"];
                        if (n.contains("position"))
                            defBodyPos = Vector3(n["position"][0].get<float>(), n["position"][1].get<float>(), n["position"][2].get<float>());
                        if (n.contains("scale"))
                            defBodyScale = Vector3(n["scale"][0].get<float>(), n["scale"][1].get<float>(), n["scale"][2].get<float>());
                        if (n.contains("color"))
                            defBodyColor = Vector4(
                                n["color"][0].get<float>() / 255.0f,
                                n["color"][1].get<float>() / 255.0f,
                                n["color"][2].get<float>() / 255.0f,
                                n["color"].size() > 3 ? n["color"][3].get<float>() / 255.0f : 1.0f);
                    }
                }

                const auto& msgArray = j["layout"]["messages"];
                for (const auto& msg : msgArray)
                {
                    MessageEntry entry;
                    entry.bodyPosition = defBodyPos;
                    entry.bodyScale    = defBodyScale;
                    entry.bodyColor    = defBodyColor;

                    auto parseImageEntry = [](const nlohmann::json& src) -> MessageEntry::ImageEntry
                    {
                        MessageEntry::ImageEntry e;
                        if (src.is_string())
                        {
                            e.name = src.get<std::string>();
                        }
                        else
                        {
                            e.name = src["name"].get<std::string>();
                            if (src.contains("position"))
                            {
                                e.hasPosition = true;
                                e.position = Vector3(src["position"][0].get<float>(), src["position"][1].get<float>(), src["position"][2].get<float>());
                            }
                            if (src.contains("scale"))
                            {
                                e.hasScale = true;
                                e.scale = Vector3(src["scale"][0].get<float>(), src["scale"][1].get<float>(), src["scale"][2].get<float>());
                            }
                        }
                        return e;
                    };

                    if (msg.is_string())
                    {
                        entry.body = Utf8ToWide(msg.get<std::string>());
                    }
                    else
                    {
                        if (msg.contains("title"))
                            entry.title = Utf8ToWide(msg["title"].get<std::string>());
                        if (msg.contains("body"))
                            entry.body = Utf8ToWide(msg["body"].get<std::string>());
                        else if (msg.contains("text"))
                            entry.body = Utf8ToWide(msg["text"].get<std::string>());

                        // body position/scale/color per-message overrides
                        // supports both new "bodyPosition" key and legacy "position" key
                        if (msg.contains("bodyPosition"))
                        {
                            const auto& p = msg["bodyPosition"];
                            entry.bodyPosition = Vector3(p[0].get<float>(), p[1].get<float>(), p[2].get<float>());
                        }
                        else if (msg.contains("position"))
                        {
                            const auto& p = msg["position"];
                            entry.bodyPosition = Vector3(p[0].get<float>(), p[1].get<float>(), p[2].get<float>());
                        }
                        if (msg.contains("bodyScale"))
                        {
                            const auto& s = msg["bodyScale"];
                            entry.bodyScale = Vector3(s[0].get<float>(), s[1].get<float>(), s[2].get<float>());
                        }
                        else if (msg.contains("scale"))
                        {
                            const auto& s = msg["scale"];
                            entry.bodyScale = Vector3(s[0].get<float>(), s[1].get<float>(), s[2].get<float>());
                        }
                        if (msg.contains("bodyColor"))
                        {
                            const auto& c = msg["bodyColor"];
                            entry.bodyColor = Vector4(
                                c[0].get<float>() / 255.0f,
                                c[1].get<float>() / 255.0f,
                                c[2].get<float>() / 255.0f,
                                c.size() > 3 ? c[3].get<float>() / 255.0f : 1.0f);
                        }
                        else if (msg.contains("color"))
                        {
                            const auto& c = msg["color"];
                            entry.bodyColor = Vector4(
                                c[0].get<float>() / 255.0f,
                                c[1].get<float>() / 255.0f,
                                c[2].get<float>() / 255.0f,
                                c.size() > 3 ? c[3].get<float>() / 255.0f : 1.0f);
                        }

                        if (msg.contains("windowPosition"))
                        {
                            entry.hasWindowPosition = true;
                            const auto& p = msg["windowPosition"];
                            entry.windowPosition = Vector3(p[0].get<float>(), p[1].get<float>(), p[2].get<float>());
                        }

                        if (msg.contains("titlePosition"))
                        {
                            entry.hasTitlePosition = true;
                            const auto& p = msg["titlePosition"];
                            entry.titlePosition = Vector3(p[0].get<float>(), p[1].get<float>(), p[2].get<float>());
                        }
                        if (msg.contains("titleScale"))
                        {
                            entry.hasTitleScale = true;
                            const auto& s = msg["titleScale"];
                            entry.titleScale = Vector3(s[0].get<float>(), s[1].get<float>(), s[2].get<float>());
                        }

                        if (msg.contains("timer"))
                        {
                            entry.hasTimer = true;
                            const auto& t = msg["timer"];
                            if (t.contains("requiredSeconds"))
                                entry.timerRequired = t["requiredSeconds"].get<float>();
                            if (t.contains("position"))
                            {
                                entry.hasTimerPosition = true;
                                entry.timerPosition = Vector3(t["position"][0].get<float>(), t["position"][1].get<float>(), t["position"][2].get<float>());
                            }
                            if (t.contains("scale"))
                            {
                                entry.hasTimerScale = true;
                                entry.timerScale = Vector3(t["scale"][0].get<float>(), t["scale"][1].get<float>(), t["scale"][2].get<float>());
                            }
                        }

                        if (msg.contains("counterPosition"))
                        {
                            entry.hasCounterPosition = true;
                            const auto& p = msg["counterPosition"];
                            entry.counterPosition = Vector3(p[0].get<float>(), p[1].get<float>(), p[2].get<float>());
                        }
                        if (msg.contains("counterScale"))
                        {
                            entry.hasCounterScale = true;
                            const auto& s = msg["counterScale"];
                            entry.counterScale = Vector3(s[0].get<float>(), s[1].get<float>(), s[2].get<float>());
                        }

                        if (msg.contains("counter"))
                        {
                            entry.hasCounter = true;
                            const auto& c = msg["counter"];
                            if (c.contains("requiredCount"))
                                entry.counterRequired = c["requiredCount"].get<int>();
                            if (c.contains("button"))
                                entry.counterButton = c["button"].get<std::string>();
                            if (c.contains("unit"))
                                entry.counterUnit = c["unit"].get<std::string>();
                            if (c.contains("position"))
                            {
                                entry.hasCounterPosition = true;
                                entry.counterPosition = Vector3(c["position"][0].get<float>(), c["position"][1].get<float>(), c["position"][2].get<float>());
                            }
                            if (c.contains("scale"))
                            {
                                entry.hasCounterScale = true;
                                entry.counterScale = Vector3(c["scale"][0].get<float>(), c["scale"][1].get<float>(), c["scale"][2].get<float>());
                            }
                        }

                        if (msg.contains("enemyCounter"))
                        {
                            entry.hasEnemyCounter = true;
                            const auto& ec = msg["enemyCounter"];
                            if (ec.contains("total"))
                                entry.enemyCounterTotal = ec["total"].get<int>();
                            if (ec.contains("unit"))
                                entry.enemyCounterUnit = ec["unit"].get<std::string>();
                            if (ec.contains("position"))
                            {
                                entry.hasEnemyCounterPosition = true;
                                entry.enemyCounterPosition = Vector3(ec["position"][0].get<float>(), ec["position"][1].get<float>(), ec["position"][2].get<float>());
                            }
                            if (ec.contains("scale"))
                            {
                                entry.hasEnemyCounterScale = true;
                                entry.enemyCounterScale = Vector3(ec["scale"][0].get<float>(), ec["scale"][1].get<float>(), ec["scale"][2].get<float>());
                            }
                        }

                        if (msg.contains("enableEnemyMove") && msg["enableEnemyMove"].get<bool>())
                            entry.enableEnemyMove = true;
                        if (msg.contains("startPracticePhase") && msg["startPracticePhase"].get<bool>())
                            entry.startPracticePhase = true;
                        if (msg.contains("freezeGame") && msg["freezeGame"].get<bool>())
                            entry.freezeGame = true;
                        if (msg.contains("nextButtonHint"))
                        {
                            const auto& hint = msg["nextButtonHint"];
                            if (hint.is_boolean())
                            {
                                entry.nextButtonHint = hint.get<bool>();
                            }
                            else if (hint.is_object())
                            {
                                entry.nextButtonHint = true;
                                if (hint.contains("iconName"))
                                    entry.nextIconName = hint["iconName"].get<std::string>();
                                if (hint.contains("iconPosition"))
                                {
                                    entry.hasNextIconPosition = true;
                                    entry.nextIconPosition = Vector3(hint["iconPosition"][0].get<float>(), hint["iconPosition"][1].get<float>(), hint["iconPosition"][2].get<float>());
                                }
                                if (hint.contains("iconScale"))
                                {
                                    entry.hasNextIconScale = true;
                                    entry.nextIconScale = Vector3(hint["iconScale"][0].get<float>(), hint["iconScale"][1].get<float>(), hint["iconScale"][2].get<float>());
                                }
                                if (hint.contains("textPosition"))
                                {
                                    entry.hasNextTextPosition = true;
                                    entry.nextTextPosition = Vector3(hint["textPosition"][0].get<float>(), hint["textPosition"][1].get<float>(), hint["textPosition"][2].get<float>());
                                }
                                if (hint.contains("textScale"))
                                {
                                    entry.hasNextTextScale = true;
                                    entry.nextTextScale = Vector3(hint["textScale"][0].get<float>(), hint["textScale"][1].get<float>(), hint["textScale"][2].get<float>());
                                }
                                if (hint.contains("hideText") && hint["hideText"].get<bool>())
                                    entry.hideNextText = true;
                            }
                        }

                        if (msg.contains("images"))
                            for (const auto& img : msg["images"])
                                entry.images.push_back(parseImageEntry(img));
                        else if (msg.contains("image"))
                            entry.images.push_back(parseImageEntry(msg["image"]));

                        if (msg.contains("hideWindow") && msg["hideWindow"].get<bool>())
                            entry.hideWindow = true;
                        if (msg.contains("showLvPopupAfter") && !msg["showLvPopupAfter"].is_null())
                        {
                            const auto& lv = msg["showLvPopupAfter"];
                            if ((lv.is_boolean() && lv.get<bool>()) || lv.is_object())
                            {
                                entry.showLvPopupAfter = true;
                                if (lv.is_object())
                                {
                                    if (lv.contains("images"))
                                    {
                                        for (const auto& img : lv["images"])
                                        {
                                            MessageEntry::LvPopupImageEntry e;
                                            e.name = img["name"].get<std::string>();
                                            if (img.contains("position"))
                                            {
                                                e.hasPosition = true;
                                                e.position = Vector3(img["position"][0].get<float>(), img["position"][1].get<float>(), img["position"][2].get<float>());
                                            }
                                            if (img.contains("scale"))
                                            {
                                                e.hasScale = true;
                                                e.scale = Vector3(img["scale"][0].get<float>(), img["scale"][1].get<float>(), img["scale"][2].get<float>());
                                            }
                                            if (img.contains("overlays"))
                                            {
                                                for (const auto& ov : img["overlays"])
                                                {
                                                    MessageEntry::LvPopupImageEntry::OverlayEntry oe;
                                                    oe.name = ov["name"].get<std::string>();
                                                    if (ov.contains("position"))
                                                    {
                                                        oe.hasPosition = true;
                                                        oe.position = Vector3(ov["position"][0].get<float>(), ov["position"][1].get<float>(), ov["position"][2].get<float>());
                                                    }
                                                    if (ov.contains("scale"))
                                                    {
                                                        oe.hasScale = true;
                                                        oe.scale = Vector3(ov["scale"][0].get<float>(), ov["scale"][1].get<float>(), ov["scale"][2].get<float>());
                                                    }
                                                    e.overlays.push_back(oe);
                                                }
                                            }
                                            entry.lvPopupImages.push_back(e);
                                        }
                                    }
                                    if (lv.contains("anim"))
                                    {
                                        const auto& a = lv["anim"];
                                        entry.hasLvPopupAnim = true;
                                        if (a.contains("scale"))      entry.lvPopupAnimScale     = a["scale"].get<float>();
                                        if (a.contains("baseScale"))  entry.lvPopupAnimBaseScale = a["baseScale"].get<float>();
                                        if (a.contains("inDuration")) entry.lvPopupAnimInDur     = a["inDuration"].get<float>();
                                        if (a.contains("center"))
                                        {
                                            entry.hasLvPopupAnimCenter = true;
                                            entry.lvPopupAnimCenter = Vector3(
                                                a["center"][0].get<float>(),
                                                a["center"][1].get<float>(),
                                                a["center"][2].get<float>());
                                        }
                                    }
                                }
                            }
                        }

                        if (msg.contains("windowAnim"))
                        {
                            const auto& wa = msg["windowAnim"];
                            entry.hasWindowAnim = true;
                            if (wa.contains("scale"))
                                entry.windowAnimScale     = wa["scale"].get<float>();
                            if (wa.contains("baseScale"))
                                entry.windowAnimBaseScale = wa["baseScale"].get<float>();
                            if (wa.contains("inDuration"))
                                entry.windowAnimInDur  = wa["inDuration"].get<float>();
                            if (wa.contains("center"))
                            {
                                entry.hasWindowAnimCenter = true;
                                entry.windowAnimCenter = Vector3(
                                    wa["center"][0].get<float>(),
                                    wa["center"][1].get<float>(),
                                    wa["center"][2].get<float>());
                            }
                        }
                    }

                    messages_.push_back(entry);
                }
            }

            if (savedIndex > 0 && savedIndex < static_cast<int>(messages_.size()))
                currentIndex_ = savedIndex;

            ShowCurrentMessage();

            skipInput_ = true;

#ifdef APP_ENABLE_LAYOUT_HOTRELOAD
            {
                struct stat mst;
                if (stat("Assets/ui/layout/tutorialMessages.json", &mst) == 0)
                    messagesFileTime_ = mst.st_mtime;
            }
#endif
        }

        void TutorialMessageMenu::Update()
        {
            // ロジックで localPosition / color.w を変更した後に MenuBase::Update() を呼ぶことで
            // fontRender_ / spriteRender_ がその同フレームの位置を正しく反映する
            UpdateLogic();
            MenuBase::Update();
        }

        void TutorialMessageMenu::UpdateLogic()
        {
#ifdef APP_ENABLE_LAYOUT_HOTRELOAD
            {
                struct stat st;
                if (stat("Assets/ui/layout/tutorialMessages.json", &st) == 0 &&
                    messagesFileTime_ != st.st_mtime)
                {
                    messagesFileTime_ = st.st_mtime;
                    InitializeLogic();
                }
            }
#endif

            // レベルアップ説明ポップアップ（最優先）
            if (lvPopupState_ != LvPopupState::None)
            {
                const bool isPaused = app::core::PauseManager::IsAvailable() && app::core::PauseManager::Get().IsPause();
                if (!isPaused)
                    UpdateLvPopup(g_gameTime->GetFrameDeltaTime());
                return;
            }

            // CLEARアニメーション中はすべての入力処理をブロック
            if (clearAnimState_ != ClearAnimState::None)
            {
                const bool isPaused = app::core::PauseManager::IsAvailable() && app::core::PauseManager::Get().IsPause();
                if (!isPaused)
                    UpdateClearAnimation(g_gameTime->GetFrameDeltaTime());
                return;
            }

            if (isAllShown_ || messages_.empty()) return;

            if (app::core::PauseManager::IsAvailable() && app::core::PauseManager::Get().IsPause())
                return;

            if (skipInput_)
            {
                skipInput_ = false;
                return;
            }

            // スライドIn中は入力をブロック
            if (slideAnim_ == SlideAnim::In)
            {
                slideAnimTimer_ += g_gameTime->GetFrameDeltaTime();
                float t = slideAnimTimer_ / kSlideDur;
                if (t >= 1.0f)
                {
                    ApplySlideAnim(1.0f, true);
                    slideAnim_ = SlideAnim::None;
                    slideElems_.clear();
                }
                else
                {
                    ApplySlideAnim(t, true);
                }
                return;
            }

            // ウィンドウ In アニメーション中は入力をブロック
            if (winAnim_ == WinAnim::In)
            {
                winAnimTimer_ += g_gameTime->GetFrameDeltaTime();
                float progress = winAnimTimer_ / winAnimInDur_;
                if (progress >= 1.0f) { progress = 1.0f; winAnim_ = WinAnim::Hold; }
                ApplyWindowAnimFactor(winAnimScale_ * (kWinAnimInitialRatio + (1.0f - kWinAnimInitialRatio) * progress));
                return;
            }

            // スライドOut中: タイマー/カウンター有無に関わらず進行させて完了後に AdvanceMessage
            if (slideAnim_ == SlideAnim::Out)
            {
                slideAnimTimer_ += g_gameTime->GetFrameDeltaTime();
                float t = slideAnimTimer_ / kSlideDur;
                if (t >= 1.0f)
                {
                    // 次メッセージのスライドInが正しいbaseAlpha・basePositionを読めるよう元に戻す
                    for (const auto& s : slideElems_)
                    {
                        if (s.icon)
                        {
                            s.icon->color.w = s.baseAlpha;
                            s.icon->transform.localPosition = s.basePos;
                        }
                        else if (s.text)
                        {
                            s.text->color.w = s.baseAlpha;
                            s.text->transform.localPosition = s.basePos;
                        }
                    }
                    slideAnim_ = SlideAnim::None;
                    slideElems_.clear();
                    AdvanceMessage();
                    return;
                }
                ApplySlideAnim(t, false);
                return;
            }

            const auto& current = messages_[currentIndex_];

            // 敵撃破カウンター表示を毎フレーム更新（進行条件ではなく表示のみ）
            if (current.hasEnemyCounter && timerText_ && timerText_->isDraw)
            {
                if (app::battle::BattleManager::IsAvailable())
                {
                    int active   = app::battle::BattleManager::Get().GetTutorialActiveEnemyCount();
                    int defeated = current.enemyCounterTotal - active;
                    if (defeated < 0) defeated = 0;
                    UpdateEnemyCounterDisplay(defeated, current.enemyCounterTotal, current.enemyCounterUnit);
                }
            }

            if (current.hasTimer)
            {
                const float lx = g_pad[0]->GetLStickXF();
                const float ly = g_pad[0]->GetLStickYF();
                if (lx * lx + ly * ly > 0.09f)   // deadzone 0.3f^2
                {
                    timerElapsed_ += g_gameTime->GetFrameDeltaTime();
                    UpdateTimerDisplay(timerElapsed_);
                }
                else
                {
                    timerElapsed_ = 0.0f;
                    UpdateTimerDisplay(timerElapsed_);
                }
                if (timerElapsed_ >= current.timerRequired)
                {
                    StartClearAnimation(true);
                }
            }
            else if (current.hasCounter)
            {
                bool triggered = false;
                if (app::battle::BattleManager::IsAvailable())
                {
                    if (current.counterButton == "A")
                        triggered = app::battle::BattleManager::Get().CheckAndConsumeChargeAttackCompleted();
                    else if (current.counterButton == "Guard")
                        triggered = app::battle::BattleManager::Get().CheckAndConsumeGuardSucceeded();
                    else if (current.counterButton == "Dodge")
                        triggered = app::battle::BattleManager::Get().CheckAndConsumeAvoidSucceeded();
                    else
                        triggered = app::battle::BattleManager::Get().CheckAndConsumeComboAttackCompleted();
                }

                if (triggered)
                {
                    counterElapsed_++;
                    UpdateCounterDisplay(counterElapsed_);
                }
                if (counterElapsed_ >= current.counterRequired)
                {
                    StartClearAnimation(true);
                }
            }
            else
            {
                if (current.nextButtonHint && g_pad[0]->IsTrigger(enButtonA))
                {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonDecision));
                    if (winAnim_ == WinAnim::Hold)
                    {
                        winAnim_ = WinAnim::None;
                        RestoreWindowElems();
                        AdvanceMessage();
                    }
                    else
                    {
                        StartSlideAnim(false);
                    }
                }
            }
        }

        void TutorialMessageMenu::AdvanceMessage()
        {
            // このメッセージがゲームをフリーズしていたなら、離れる前に解除する
            if (currentIndex_ >= 0 && currentIndex_ < static_cast<int>(messages_.size()))
            {
                if (messages_[currentIndex_].freezeGame && app::battle::BattleManager::IsAvailable())
                    app::battle::BattleManager::Get().SetTutorialFreeze(false);
                // 「チュートリアルをクリアしよう！」へ移行するタイミングでダメージ無効を解除する
                if (messages_[currentIndex_].startPracticePhase && app::battle::BattleManager::IsAvailable())
                    app::battle::BattleManager::Get().SetTutorialNoDamage(false);

                // このメッセージ完了後にレベル/フェーズ説明ポップアップを表示する
                if (messages_[currentIndex_].showLvPopupAfter)
                {
                    lvPopupPendingAdvance_ = true;
                    lvPopupStep_ = 0;
                    StartLvPopup();
                    return;
                }
            }

            currentIndex_++;

            if (currentIndex_ >= static_cast<int>(messages_.size()))
            {
                isAllShown_ = true;
                return;
            }

            ShowCurrentMessage();
        }

        void TutorialMessageMenu::ShowCurrentMessage()
        {
            if (currentIndex_ < 0 || currentIndex_ >= static_cast<int>(messages_.size())) return;

            if (messages_[currentIndex_].startPracticePhase)
                practicePhaseReached_ = true;
            if (messages_[currentIndex_].freezeGame && app::battle::BattleManager::IsAvailable())
                app::battle::BattleManager::Get().SetTutorialFreeze(true);

            for (UIIcon* img : currentTutorialImages_)
                if (img) img->isDraw = false;
            currentTutorialImages_.clear();

            const auto& entry = messages_[currentIndex_];

            if (messageWindowIcon_)         messageWindowIcon_->isDraw         = !entry.startPracticePhase && !entry.hideWindow;
            if (messageWindowPracticeIcon_) messageWindowPracticeIcon_->isDraw = entry.startPracticePhase && !entry.hideWindow;

            if (entry.hasWindowPosition)
            {
                UIIcon* win = entry.startPracticePhase ? messageWindowPracticeIcon_ : messageWindowIcon_;
                if (win) win->transform.localPosition = entry.windowPosition;
            }

            if (titleText_)
            {
                titleText_->SetText(entry.title.c_str());
                titleText_->transform.localPosition = entry.hasTitlePosition ? entry.titlePosition : defaultTitlePosition_;
                titleText_->transform.localScale    = entry.hasTitleScale    ? entry.titleScale    : defaultTitleScale_;
                titleText_->isDraw = !entry.title.empty();
            }

            if (titleLineIcon_)
                titleLineIcon_->isDraw         = !entry.title.empty() && !entry.startPracticePhase;
            if (titleLinePracticeIcon_)
                titleLinePracticeIcon_->isDraw = !entry.title.empty() &&  entry.startPracticePhase;

            if (messageText_)
            {
                messageText_->SetText(entry.body.c_str());
                messageText_->transform.localPosition = entry.bodyPosition;
                messageText_->transform.localScale    = entry.bodyScale;
                messageText_->color                   = entry.bodyColor;
                messageText_->isDraw                  = true;
            }

            if (timerText_)
            {
                timerText_->isDraw = entry.hasTimer || entry.hasCounter || entry.hasEnemyCounter;
                if (entry.hasEnemyCounter)
                {
                    timerText_->transform.localPosition = entry.hasEnemyCounterPosition ? entry.enemyCounterPosition : defaultTimerPosition_;
                    timerText_->transform.localScale    = entry.hasEnemyCounterScale    ? entry.enemyCounterScale    : defaultTimerScale_;
                    UpdateEnemyCounterDisplay(0, entry.enemyCounterTotal, entry.enemyCounterUnit);
                }
                else if (entry.hasTimer)
                {
                    timerElapsed_ = 0.0f;
                    timerText_->transform.localPosition = entry.hasTimerPosition ? entry.timerPosition : defaultTimerPosition_;
                    timerText_->transform.localScale    = entry.hasTimerScale    ? entry.timerScale    : defaultTimerScale_;
                    UpdateTimerDisplay(0.0f);
                }
                else if (entry.hasCounter)
                {
                    counterElapsed_ = 0;
                    // メッセージ切り替え前の通知を排出（誤カウント防止）
                    if (app::battle::BattleManager::IsAvailable())
                    {
                        if (entry.counterButton == "A")
                            app::battle::BattleManager::Get().CheckAndConsumeChargeAttackCompleted();
                        else if (entry.counterButton == "Guard")
                            app::battle::BattleManager::Get().CheckAndConsumeGuardSucceeded();
                        else if (entry.counterButton == "Dodge")
                            app::battle::BattleManager::Get().CheckAndConsumeAvoidSucceeded();
                        else
                            app::battle::BattleManager::Get().CheckAndConsumeComboAttackCompleted();
                    }
                    timerText_->transform.localPosition = entry.hasCounterPosition ? entry.counterPosition : defaultTimerPosition_;
                    timerText_->transform.localScale    = entry.hasCounterScale    ? entry.counterScale   : defaultTimerScale_;
                    UpdateCounterDisplay(0);
                }

            if (entry.enableEnemyMove && app::battle::BattleManager::IsAvailable())
                app::battle::BattleManager::Get().SetTutorialEnemyMoveEnabled(true);
            }

            if (nextButtonAltIcon_)
            {
                nextButtonAltIcon_->isDraw = false;
                nextButtonAltIcon_ = nullptr;
            }
            if (nextButtonIcon_)
                nextButtonIcon_->isDraw = false;

            if (entry.nextButtonHint)
            {
                UIIcon* activeIcon = nullptr;
                if (!entry.nextIconName.empty())
                {
                    nextButtonAltIcon_ = GetUI<UIIcon>(Hash32(entry.nextIconName.c_str()));
                    activeIcon = nextButtonAltIcon_;
                }
                else
                {
                    activeIcon = nextButtonIcon_;
                }
                if (activeIcon)
                {
                    if (entry.hasNextIconPosition) activeIcon->transform.localPosition = entry.nextIconPosition;
                    if (entry.hasNextIconScale)    activeIcon->transform.localScale    = entry.nextIconScale;
                    activeIcon->isDraw = true;
                }
            }
            if (nextHintText_)
            {
                nextHintText_->isDraw = entry.nextButtonHint && !entry.hideNextText;
                if (entry.nextButtonHint && !entry.hideNextText)
                {
                    if (entry.hasNextTextPosition) nextHintText_->transform.localPosition = entry.nextTextPosition;
                    if (entry.hasNextTextScale)    nextHintText_->transform.localScale    = entry.nextTextScale;
                }
            }

            for (const auto& imgEntry : entry.images)
            {
                UIIcon* img = GetUI<UIIcon>(Hash32(imgEntry.name.c_str()));
                if (img)
                {
                    if (imgEntry.hasPosition)
                        img->transform.localPosition = imgEntry.position;
                    if (imgEntry.hasScale)
                        img->transform.localScale = imgEntry.scale;
                    img->isDraw = true;
                    currentTutorialImages_.push_back(img);
                }
            }

            if (entry.hasWindowAnim)
            {
                StartWindowAnim(entry);
                slideAnim_ = SlideAnim::None;
                slideElems_.clear();
            }
            else
            {
                winAnim_ = WinAnim::None;
                if (!entry.startPracticePhase)
                    StartSlideAnim(true);
                else
                {
                    slideAnim_ = SlideAnim::None;
                    slideElems_.clear();
                }
            }
        }

        void TutorialMessageMenu::SetTimerProgress(float elapsed)
        {
            if (!timerText_ || !timerText_->isDraw) return;
            UpdateTimerDisplay(elapsed);
        }

        void TutorialMessageMenu::UpdateTimerDisplay(float elapsed)
        {
            if (!timerText_) return;
            if (currentIndex_ < 0 || currentIndex_ >= static_cast<int>(messages_.size())) return;

            const auto& entry = messages_[currentIndex_];
            if (!entry.hasTimer) return;

            int e = static_cast<int>(elapsed);
            int r = static_cast<int>(entry.timerRequired);
            std::wstring timerStr = L"(" + std::to_wstring(e) + L"/" + std::to_wstring(r) + L"\u79d2)";
            timerText_->SetText(timerStr.c_str());
        }

        void TutorialMessageMenu::UpdateCounterDisplay(int current)
        {
            if (!timerText_) return;
            if (currentIndex_ < 0 || currentIndex_ >= static_cast<int>(messages_.size())) return;
            const auto& entry = messages_[currentIndex_];
            if (!entry.hasCounter) return;
            std::wstring unit = Utf8ToWide(entry.counterUnit);
            std::wstring str = L"(" + std::to_wstring(current) + L"/" + std::to_wstring(entry.counterRequired) + unit + L")";
            timerText_->SetText(str.c_str());
        }

        void TutorialMessageMenu::UpdateEnemyCounterDisplay(int defeated, int total, const std::string& unit)
        {
            if (!timerText_) return;
            std::wstring unitW = Utf8ToWide(unit);
            std::wstring str = L"(" + std::to_wstring(defeated) + L"/" + std::to_wstring(total) + unitW + L")";
            timerText_->SetText(str.c_str());
        }

        void TutorialMessageMenu::TriggerFinalClear()
        {
            // 既にアニメーション中または完了済みなら無視
            if (finalClearDone_ || clearAnimState_ != ClearAnimState::None) return;
            StartClearAnimation(false);
        }

        void TutorialMessageMenu::StartClearAnimation(bool pendingAdvance)
        {
            if (slideAnim_ == SlideAnim::In)
                ApplySlideAnim(1.0f, true);
            slideAnim_ = SlideAnim::None;
            slideElems_.clear();

            clearPendingAdv_ = pendingAdvance;
            clearAnimState_  = ClearAnimState::ZoomIn;
            clearAnimTimer_  = 0.0f;

            if (clearBannerIcon_) { clearBannerIcon_->isDraw = true;  clearBannerIcon_->color.w = 1.0f; }
            if (clearTextIcon_)   { clearTextIcon_->isDraw   = true;  clearTextIcon_->color.w   = 1.0f; }
            ApplyClearScale(kClearScaleMax, 1.0f);
        }

        void TutorialMessageMenu::ApplyClearScale(float factor, float alpha)
        {
            if (clearBannerIcon_)
            {
                clearBannerIcon_->transform.localScale = Vector3(
                    clearBannerBaseScale_.x * factor,
                    clearBannerBaseScale_.y * factor,
                    clearBannerBaseScale_.z);
                clearBannerIcon_->color.w = alpha;
            }
            if (clearTextIcon_)
            {
                clearTextIcon_->transform.localScale = Vector3(
                    clearTextBaseScale_.x * factor,
                    clearTextBaseScale_.y * factor,
                    clearTextBaseScale_.z);
                clearTextIcon_->color.w = alpha;
            }
        }

        void TutorialMessageMenu::UpdateClearAnimation(float dt)
        {
            clearAnimTimer_ += dt;

            switch (clearAnimState_)
            {
            case ClearAnimState::ZoomIn:
            {
                float t = clearAnimTimer_ / kClearZoomInDur;
                if (t >= 1.0f) { t = 1.0f; clearAnimState_ = ClearAnimState::Hold; clearAnimTimer_ = 0.0f; }
                // kClearScaleMax(2.0) → 1.0 に縮小しながら登場
                float factor = kClearScaleMax + (1.0f - kClearScaleMax) * t;
                ApplyClearScale(factor, 1.0f);
                break;
            }
            case ClearAnimState::Hold:
            {
                if (clearAnimTimer_ >= kClearHoldDur) { clearAnimState_ = ClearAnimState::ZoomOut; clearAnimTimer_ = 0.0f; }
                ApplyClearScale(1.0f, 1.0f);
                break;
            }
            case ClearAnimState::ZoomOut:
            {
                float t = clearAnimTimer_ / kClearFadeOutDur;
                if (t >= 1.0f)
                {
                    if (clearBannerIcon_) clearBannerIcon_->isDraw = false;
                    if (clearTextIcon_)   clearTextIcon_->isDraw   = false;

                    if (clearPendingAdv_)
                    {
                        clearPendingAdv_ = false;
                        clearAnimState_ = ClearAnimState::None;
                        StartSlideAnim(false);
                    }
                    else
                    {
                        StartSlideAnim(false);
                        slideAnim_ = SlideAnim::None;
                        clearAnimState_ = ClearAnimState::FinalSlideOut;
                    }
                    return;
                }
                ApplyClearScale(1.0f, 1.0f - t);
                break;
            }
            case ClearAnimState::FinalSlideOut:
            {
                slideAnimTimer_ += dt;
                float t = slideAnimTimer_ / kSlideDur;
                if (t >= 1.0f)
                {
                    ApplySlideAnim(1.0f, false);
                    for (const auto& s : slideElems_)
                    {
                        if (s.icon)
                        {
                            s.icon->color.w  = s.baseAlpha;
                            s.icon->transform.localPosition = s.basePos;
                            s.icon->isDraw   = false;
                        }
                        else if (s.text)
                        {
                            s.text->color.w  = s.baseAlpha;
                            s.text->transform.localPosition = s.basePos;
                            s.text->isDraw   = false;
                        }
                    }
                    slideElems_.clear();
                    clearAnimState_ = ClearAnimState::None;
                    finalClearDone_ = true;
                    return;
                }
                ApplySlideAnim(t, false);
                break;
            }
            default: break;
            }
        }

        void TutorialMessageMenu::StartWindowAnim(const MessageEntry& entry)
        {
            // ホットリロードで再呼び出しされてもスケール・位置が累積しないよう、
            // 前回記録したレイアウト由来のスケールと位置に戻してからアニメを開始する
            // テキスト要素は AdvanceMessage で位置が毎回セットされるためスケールのみ復元する
            for (const auto& orig : winOrigScales_)
            {
                if (orig.icon) { orig.icon->transform.localScale    = orig.scale;
                                 orig.icon->transform.localPosition = orig.pos; }
                if (orig.text) { orig.text->transform.localScale    = orig.scale; }
            }
            winOrigScales_.clear();

            winElems_.clear();
            winAnimScale_     = entry.windowAnimScale;
            winAnimBaseScale_ = entry.windowAnimBaseScale;
            winAnimInDur_     = entry.windowAnimInDur;

            // ウィンドウ背景の現在位置を基準に、中心点への移動量を計算
            Vector3 defaultPos = messageWindowIcon_
                ? messageWindowIcon_->transform.localPosition
                : Vector3::Zero;
            winAnimCenter_ = entry.hasWindowAnimCenter ? entry.windowAnimCenter : defaultPos;
            Vector3 shift  = winAnimCenter_ - defaultPos;

            auto snapIcon = [&](UIIcon* icon)
            {
                if (!icon || !icon->isDraw) return;
                winOrigScales_.push_back({icon, nullptr, icon->transform.localScale, icon->transform.localPosition});
                WinElemSnap s;
                s.icon      = icon;
                s.origPos   = icon->transform.localPosition;
                s.baseScale = icon->transform.localScale;
                icon->transform.localPosition = icon->transform.localPosition + shift;
                s.targetPos = icon->transform.localPosition;
                winElems_.push_back(s);
            };
            auto snapText = [&](UIText* text)
            {
                if (!text || !text->isDraw) return;
                winOrigScales_.push_back({nullptr, text, text->transform.localScale, text->transform.localPosition});
                WinElemSnap s;
                s.text      = text;
                s.origPos   = text->transform.localPosition;
                s.baseScale = text->transform.localScale;
                text->transform.localPosition = text->transform.localPosition + shift;
                s.targetPos = text->transform.localPosition;
                winElems_.push_back(s);
            };

            snapIcon(messageWindowIcon_);
            snapIcon(messageWindowPracticeIcon_);
            snapIcon(titleLineIcon_);
            snapIcon(titleLinePracticeIcon_);
            snapText(titleText_);
            snapText(messageText_);
            snapText(timerText_);
            UIIcon* nextIcon = nextButtonAltIcon_ ? nextButtonAltIcon_ : nextButtonIcon_;
            snapIcon(nextIcon);
            snapText(nextHintText_);
            for (UIIcon* img : currentTutorialImages_)
                snapIcon(img);

            // baseScale をスナップショットに乗算して初期サイズを拡大
            if (winAnimBaseScale_ != 1.0f)
            {
                for (auto& s : winElems_)
                {
                    s.baseScale = Vector3(
                        s.baseScale.x * winAnimBaseScale_,
                        s.baseScale.y * winAnimBaseScale_,
                        s.baseScale.z);
                    Vector3 offset = s.targetPos - winAnimCenter_;
                    s.targetPos = winAnimCenter_ + Vector3(
                        offset.x * winAnimBaseScale_,
                        offset.y * winAnimBaseScale_,
                        offset.z);
                }
            }

            winAnim_      = WinAnim::In;
            winAnimTimer_ = 0.0f;
            ApplyWindowAnimFactor(winAnimScale_ * kWinAnimInitialRatio);
        }

        void TutorialMessageMenu::ApplyWindowAnimFactor(float t)
        {
            const float norm = (winAnimScale_ > 0.0f) ? (t / winAnimScale_) : 0.0f;
            for (const auto& s : winElems_)
            {
                Vector3 animPos   = winAnimCenter_ + (s.targetPos - winAnimCenter_) * norm;
                Vector3 animScale = Vector3(s.baseScale.x * t, s.baseScale.y * t, s.baseScale.z);
                if (s.icon)
                {
                    s.icon->transform.localPosition = animPos;
                    s.icon->transform.localScale    = animScale;
                }
                else if (s.text)
                {
                    s.text->transform.localPosition = animPos;
                    s.text->transform.localScale    = animScale;
                }
            }
        }

        void TutorialMessageMenu::RestoreWindowElems()
        {
            for (const auto& s : winElems_)
            {
                if (s.icon)
                {
                    s.icon->transform.localPosition = s.origPos;
                    s.icon->transform.localScale    = s.baseScale;
                }
                else if (s.text)
                {
                    s.text->transform.localPosition = s.origPos;
                    s.text->transform.localScale    = s.baseScale;
                }
            }
            winElems_.clear();
        }

        void TutorialMessageMenu::StartSlideAnim(bool isIn)
        {
            slideElems_.clear();
            slideAnim_      = isIn ? SlideAnim::In : SlideAnim::Out;
            slideAnimTimer_ = 0.0f;

            auto snapIcon = [&](UIIcon* icon)
            {
                if (!icon || !icon->isDraw) return;
                SlideElemSnap s;
                s.icon      = icon;
                s.basePos   = icon->transform.localPosition;
                s.baseAlpha = icon->color.w;
                slideElems_.push_back(s);
            };
            auto snapText = [&](UIText* text)
            {
                if (!text || !text->isDraw) return;
                SlideElemSnap s;
                s.text      = text;
                s.basePos   = text->transform.localPosition;
                s.baseAlpha = text->color.w;
                slideElems_.push_back(s);
            };

            snapIcon(messageWindowIcon_);
            snapIcon(messageWindowPracticeIcon_);
            snapIcon(titleLineIcon_);
            snapIcon(titleLinePracticeIcon_);
            snapText(titleText_);
            snapText(messageText_);
            snapText(timerText_);
            UIIcon* nextIcon = nextButtonAltIcon_ ? nextButtonAltIcon_ : nextButtonIcon_;
            snapIcon(nextIcon);
            snapText(nextHintText_);
            for (UIIcon* img : currentTutorialImages_)
                snapIcon(img);

            ApplySlideAnim(0.0f, isIn);
        }

        void TutorialMessageMenu::ApplySlideAnim(float t, bool isIn)
        {
            float yOffset, alpha;
            if (isIn)
            {
                // 下 (Y - offset) → 現在位置、alpha 0 → 1
                yOffset = -kSlideOffset * (1.0f - t);
                alpha   = t;
            }
            else
            {
                // 現在位置 → 上 (Y + offset)、alpha 1 → 0
                yOffset = kSlideOffset * t;
                alpha   = 1.0f - t;
            }

            for (const auto& s : slideElems_)
            {
                Vector3 pos(s.basePos.x, s.basePos.y + yOffset, s.basePos.z);
                float   a = s.baseAlpha * alpha;
                if (s.icon)
                {
                    s.icon->transform.localPosition = pos;
                    s.icon->color.w = a;
                }
                else if (s.text)
                {
                    s.text->transform.localPosition = pos;
                    s.text->color.w = a;
                }
            }
        }

        void TutorialMessageMenu::StartLvPopup()
        {
            if (currentIndex_ < 0 || currentIndex_ >= static_cast<int>(messages_.size())) return;
            const auto& entry = messages_[currentIndex_];

            const int maxStep = entry.lvPopupImages.empty()
                ? 1
                : static_cast<int>(entry.lvPopupImages.size()) - 1;

            // ステップ0開始時: メッセージウィンドウUIを隠す
            if (lvPopupStep_ == 0)
            {
                if (messageWindowIcon_)         messageWindowIcon_->isDraw         = false;
                if (messageWindowPracticeIcon_) messageWindowPracticeIcon_->isDraw = false;
                if (titleText_)                 titleText_->isDraw                 = false;
                if (titleLineIcon_)             titleLineIcon_->isDraw             = false;
                if (titleLinePracticeIcon_)     titleLinePracticeIcon_->isDraw     = false;
                if (messageText_)               messageText_->isDraw               = false;
                if (timerText_)                 timerText_->isDraw                 = false;
                if (nextButtonIcon_)            nextButtonIcon_->isDraw            = false;
                if (nextButtonAltIcon_)         nextButtonAltIcon_->isDraw         = false;
                if (nextHintText_)              nextHintText_->isDraw              = false;
                for (UIIcon* img : currentTutorialImages_)
                    if (img) img->isDraw = false;
            }

            // 前ステップの要素をすべて非表示にしてリスト解放
            for (auto& s : lvPopupElems_)
                if (s.icon) s.icon->isDraw = false;
            lvPopupElems_.clear();

            UIIcon* icon       = nullptr;
            bool hasConfigPos  = false;
            Vector3 configPos;
            bool hasConfigScale = false;
            Vector3 configScale;
            const MessageEntry::LvPopupImageEntry* imgCfg = nullptr;

            if (!entry.lvPopupImages.empty() && lvPopupStep_ < static_cast<int>(entry.lvPopupImages.size()))
            {
                imgCfg         = &entry.lvPopupImages[lvPopupStep_];
                icon           = GetUI<UIIcon>(Hash32(imgCfg->name.c_str()));
                hasConfigPos   = imgCfg->hasPosition;
                configPos      = imgCfg->position;
                hasConfigScale = imgCfg->hasScale;
                configScale    = imgCfg->scale;
            }
            else
            {
                icon = (lvPopupStep_ == 0) ? levelExplainIcon_ : phaseExplainIcon_;
            }

            if (!icon)
            {
                if (lvPopupStep_ < maxStep)
                {
                    lvPopupStep_++;
                    StartLvPopup();
                }
                else
                {
                    lvPopupState_ = LvPopupState::None;
                    if (app::battle::BattleManager::IsAvailable())
                        app::battle::BattleManager::Get().SetTutorialFreeze(false);
                    if (lvPopupPendingAdvance_)
                    {
                        lvPopupPendingAdvance_ = false;
                        currentIndex_++;
                        if (currentIndex_ >= static_cast<int>(messages_.size()))
                            isAllShown_ = true;
                        else
                            ShowCurrentMessage();
                    }
                }
                return;
            }

            // アニメーション設定を確定
            lvPopupCurScale_     = entry.hasLvPopupAnim ? entry.lvPopupAnimScale     : kLvPopupScaleMax;
            lvPopupCurBaseScale_ = entry.hasLvPopupAnim ? entry.lvPopupAnimBaseScale : kLvPopupBaseScale;
            lvPopupCurInDur_     = entry.hasLvPopupAnim ? entry.lvPopupAnimInDur     : kLvPopupInDur;
            lvPopupCurCenter_    = (entry.hasLvPopupAnim && entry.hasLvPopupAnimCenter)
                                   ? entry.lvPopupAnimCenter
                                   : Vector3(0.0f, 0.0f, 0.0f);

            // メインアイコンを設定
            if (hasConfigPos)   icon->transform.localPosition = configPos;
            if (hasConfigScale) icon->transform.localScale    = configScale;
            icon->isDraw        = true;
            lvPopupCurrentIcon_ = icon;

            // windowAnim と同じシフト計算: shift = center - mainIconPos
            const Vector3 shift = lvPopupCurCenter_ - icon->transform.localPosition;

            // 全要素（メイン+オーバーレイ）を一括登録するヘルパー
            auto addSnap = [&](UIIcon* ic)
            {
                LvPopupElemSnap s;
                s.icon      = ic;
                s.baseScale = Vector3(
                    ic->transform.localScale.x * lvPopupCurBaseScale_,
                    ic->transform.localScale.y * lvPopupCurBaseScale_,
                    ic->transform.localScale.z);
                s.targetPos = ic->transform.localPosition + shift;
                if (lvPopupCurBaseScale_ != 1.0f)
                {
                    const Vector3 offset = s.targetPos - lvPopupCurCenter_;
                    s.targetPos = lvPopupCurCenter_ + Vector3(
                        offset.x * lvPopupCurBaseScale_,
                        offset.y * lvPopupCurBaseScale_,
                        offset.z);
                }
                lvPopupElems_.push_back(s);
            };

            addSnap(icon);

            // オーバーレイアイコンを設定してスナップ登録
            if (imgCfg)
            {
                for (const auto& ov : imgCfg->overlays)
                {
                    UIIcon* ovIcon = GetUI<UIIcon>(Hash32(ov.name.c_str()));
                    if (!ovIcon) continue;
                    if (ov.hasPosition) ovIcon->transform.localPosition = ov.position;
                    if (ov.hasScale)    ovIcon->transform.localScale    = ov.scale;
                    ovIcon->isDraw = true;
                    addSnap(ovIcon);
                }
            }

            if (app::battle::BattleManager::IsAvailable())
                app::battle::BattleManager::Get().SetTutorialFreeze(true);

            lvPopupState_ = LvPopupState::In;
            lvPopupTimer_ = 0.0f;
            ApplyLvPopupFactor(lvPopupCurScale_ * kWinAnimInitialRatio);
        }

        void TutorialMessageMenu::UpdateLvPopup(float dt)
        {
            switch (lvPopupState_)
            {
            case LvPopupState::In:
            {
                lvPopupTimer_ += dt;
                float progress = (lvPopupCurInDur_ > 0.0f) ? (lvPopupTimer_ / lvPopupCurInDur_) : 1.0f;
                if (progress >= 1.0f) { progress = 1.0f; lvPopupState_ = LvPopupState::Hold; }
                ApplyLvPopupFactor(lvPopupCurScale_ * (kWinAnimInitialRatio + (1.0f - kWinAnimInitialRatio) * progress));
                break;
            }
            case LvPopupState::Hold:
            {
                ApplyLvPopupFactor(lvPopupCurScale_);
                if (g_pad[0]->IsTrigger(enButtonA))
                {
                    app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonDecision));
                    for (auto& s : lvPopupElems_)
                        if (s.icon) s.icon->isDraw = false;

                    int maxStep = 1;
                    if (currentIndex_ >= 0 && currentIndex_ < static_cast<int>(messages_.size()))
                    {
                        const auto& e = messages_[currentIndex_];
                        if (!e.lvPopupImages.empty())
                            maxStep = static_cast<int>(e.lvPopupImages.size()) - 1;
                    }

                    if (lvPopupStep_ < maxStep)
                    {
                        lvPopupStep_++;
                        StartLvPopup();
                    }
                    else
                    {
                        lvPopupState_ = LvPopupState::None;
                        if (app::battle::BattleManager::IsAvailable())
                            app::battle::BattleManager::Get().SetTutorialFreeze(false);

                        if (lvPopupPendingAdvance_)
                        {
                            lvPopupPendingAdvance_ = false;
                            currentIndex_++;
                            if (currentIndex_ >= static_cast<int>(messages_.size()))
                                isAllShown_ = true;
                            else
                                ShowCurrentMessage();
                        }
                    }
                }
                break;
            }
            default: break;
            }
        }

        void TutorialMessageMenu::ApplyLvPopupFactor(float t)
        {
            const float norm = (lvPopupCurScale_ > 0.0f) ? (t / lvPopupCurScale_) : 0.0f;
            for (auto& s : lvPopupElems_)
            {
                if (!s.icon) continue;
                s.icon->transform.localPosition =
                    lvPopupCurCenter_ + (s.targetPos - lvPopupCurCenter_) * norm;
                s.icon->transform.localScale = Vector3(
                    s.baseScale.x * t,
                    s.baseScale.y * t,
                    s.baseScale.z);
            }
        }

        void TutorialMessageMenu::ApplyLayoutOverrides(const nlohmann::json& layout)
        {
            auto applyToIcon = [](UIIcon* icon, const nlohmann::json& node)
            {
                if (!icon) return;
                if (node.contains("position"))
                    icon->transform.localPosition = Vector3(
                        node["position"][0].get<float>(),
                        node["position"][1].get<float>(),
                        node["position"][2].get<float>());
                if (node.contains("scale"))
                    icon->transform.localScale = Vector3(
                        node["scale"][0].get<float>(),
                        node["scale"][1].get<float>(),
                        node["scale"][2].get<float>());
                if (node.contains("rotation"))
                    icon->transform.localRotation.SetRotationDegZ(node["rotation"].get<float>());
            };

            auto applyToText = [](UIText* text, const nlohmann::json& node)
            {
                if (!text) return;
                if (node.contains("position"))
                    text->transform.localPosition = Vector3(
                        node["position"][0].get<float>(),
                        node["position"][1].get<float>(),
                        node["position"][2].get<float>());
                if (node.contains("scale"))
                    text->transform.localScale = Vector3(
                        node["scale"][0].get<float>(),
                        node["scale"][1].get<float>(),
                        node["scale"][2].get<float>());
                if (node.contains("rotation"))
                    text->transform.localRotation.SetRotationDegZ(node["rotation"].get<float>());
            };

            if (layout.contains("messageWindow"))
                applyToIcon(messageWindowIcon_, layout["messageWindow"]);
            if (layout.contains("messageWindowTitle"))
            {
                const auto& n = layout["messageWindowTitle"];
                if (n.contains("position"))
                    defaultTitlePosition_ = Vector3(n["position"][0].get<float>(), n["position"][1].get<float>(), n["position"][2].get<float>());
                if (n.contains("scale"))
                    defaultTitleScale_ = Vector3(n["scale"][0].get<float>(), n["scale"][1].get<float>(), n["scale"][2].get<float>());
                applyToText(titleText_, n);
            }
            if (layout.contains("messageWindowTitleLine"))
                applyToIcon(titleLineIcon_, layout["messageWindowTitleLine"]);
            if (layout.contains("messageWindowText"))
                applyToText(messageText_, layout["messageWindowText"]);
            if (layout.contains("messageWindowTimer"))
            {
                const auto& n = layout["messageWindowTimer"];
                if (n.contains("position"))
                    defaultTimerPosition_ = Vector3(n["position"][0].get<float>(), n["position"][1].get<float>(), n["position"][2].get<float>());
                if (n.contains("scale"))
                    defaultTimerScale_ = Vector3(n["scale"][0].get<float>(), n["scale"][1].get<float>(), n["scale"][2].get<float>());
                applyToText(timerText_, n);
            }
        }
    }
}
