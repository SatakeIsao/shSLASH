#include "stdafx.h"
#include "InGameUI.h"
#include "core/ParameterManager.h"
#include "actor/ActorStatus.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"
#include "effect/EffectManager2D.h"

namespace {
	/** 色 */
	/** 通常時のゲージ色（クリーム） */
	static const Vector4 TIMER_COLOR_CREAM = { 0.761f, 0.749f, 0.620f, 1.0f };
	/** 警告時のゲージ色（オレンジ） */
	static const Vector4 TIMER_COLOR_ORANGE = { 1.0f,   0.55f,  0.0f,   1.0f };
	/** 危険時のゲージ色（赤） */
	static const Vector4 TIMER_COLOR_RED = { 1.0f,   0.1f,   0.1f,   1.0f };
	/** 経過時間エリアの色（黒） */
	static const Vector4 TIMER_COLOR_BLACK = { 0.0f,   0.0f,   0.0f,   1.0f };
	/** 経過時間エリアの色（薄いクリーム灰） */
	static const Vector4 TIMER_COLOR_EMPTY = { 0.25f, 0.25f, 0.20f, 1.0f };
	/** 外側装飾リングの色（白） */
	static const Vector4 TIMER_COLOR_WHITE = { 1.0f,   1.0f,   1.0f,   1.0f };

	/** 半径 */
	/** 内側円の外縁 = 黒区切りリングの内縁 */
	static constexpr float TIMER_BG_OUTER_R = 0.85f;
	/** 黒区切りリングの外縁 = 外側白リングの内縁 */
	static constexpr float TIMER_GAP_OUTER_R = 0.92f;

	/** 警告しきい値 */
	/** この割合以下で橙色に変化（残り30%） */
	static constexpr float TIMER_WARNING_RATIO = 0.30f;
	/** この割合以下で赤く点滅（残り10%） */
	static constexpr float TIMER_DANGER_RATIO = 0.10f;
	/** 点滅の切り替え間隔（秒） */
	static constexpr float TIMER_BLINK_SPEED = 0.08f;

	/** 座標・スケール */
	/** タイマーUIの表示座標（画面右上） */
	static const Vector3 TIMER_POSITION = { 720.0f, 340.0f, 0.0f };
	/** 内側塗りつぶし円のスケール */
	static constexpr float   TIMER_SCALE_LARGE = 0.75f;
	/** リング類のスケール */
	static constexpr float   TIMER_SCALE_NORMAL = 0.65f;

	/** 共通定数 */
	/** バトルの制限時間（秒） */
	static constexpr int MAX_TIME = 10;
	/** レベルの最大値 */
	static constexpr int MAX_LEVEL = 10;
	/** 敵撃破時に獲得する経験値
	 * @todo Lv.10に近づくにつれて獲得量を減らす調整が必要 */
	static constexpr int GOIN_EXP = 1;

	/** HPバー定数 */
	/** 敵HPバーを表示する最大距離 */
	static constexpr float DRAW_DISTANCE = 400.0f;
	/** ダメージバーが追従を開始するまでの遅延時間 */
	static constexpr float DAMAGE_DELAY_TIME = 0.5f;

	/** HPバーシェーダー用：左端の斜め幅（UV空間） */
	static constexpr float HP_BAR_LEFT_W = 0.06f;

	/** 無敵中の点滅切り替え間隔 */
	static constexpr float BLINK_INTERVAL = 0.01f;
	/** HPがこの割合以下で点滅演出を開始 */
	static constexpr float HP_BLINK_THRESHOLD = 0.3f;
}

namespace app
{
	namespace ui
	{
		TimerUIObject::TimerUIObject()
		{
			timer_ = MAX_TIME;
			maxTime_ = MAX_TIME;

			/** 内側塗りつぶし円（残り時間で色変化） */
			{
				timerBackGround_.Init(nullptr, 200.0f, 200.0f);
				timerBackGround_.SetPosition(TIMER_POSITION);
				timerBackGround_.SetScale(TIMER_SCALE_LARGE);
				timerBackGround_.SetInnerRadius(0.0f);
				timerBackGround_.SetOuterRadius(TIMER_BG_OUTER_R);
				timerBackGround_.SetStartAngle(0.0f);
				timerBackGround_.SetArcSpan(Math::PI2);
				timerBackGround_.SetFillAmount(1.0f);
				timerBackGround_.SetFillColor(TIMER_COLOR_CREAM);
				timerBackGround_.SetEmptyColor(TIMER_COLOR_BLACK);
			}
			/** 中間ゲージリング（残り時間で色変化） */
			{
				timerGauge_.Init(nullptr, 200.0f, 200.0f);
				timerGauge_.SetPosition(TIMER_POSITION);
				timerGauge_.SetScale(TIMER_SCALE_NORMAL);
				timerGauge_.SetInnerRadius(0.55f);
				timerGauge_.SetOuterRadius(TIMER_BG_OUTER_R);
				timerGauge_.SetFillAmount(1.0f);
				timerGauge_.SetStartAngle(0.0f);
				timerGauge_.SetArcSpan(Math::PI2);
				timerGauge_.SetFillColor(TIMER_COLOR_CREAM);
				timerGauge_.SetEmptyColor(TIMER_COLOR_BLACK);
			}
			/** 隙間を埋める黒リング */
			{
				timerGapRing_.Init(nullptr, 200.0f, 200.0f);
				timerGapRing_.SetPosition(TIMER_POSITION);
				timerGapRing_.SetScale(TIMER_SCALE_NORMAL);
				timerGapRing_.SetInnerRadius(TIMER_BG_OUTER_R);
				timerGapRing_.SetOuterRadius(TIMER_GAP_OUTER_R);
				timerGapRing_.SetFillAmount(1.0f);
				timerGapRing_.SetArcSpan(0.0f);
				timerGapRing_.SetFillColor(TIMER_COLOR_BLACK);
				timerGapRing_.SetEmptyColor(TIMER_COLOR_BLACK);
			}
			/** 外側白リング（固定・装飾） */
			{
				timerOuterRing_.Init(nullptr, 200.0f, 200.0f);
				timerOuterRing_.SetPosition(TIMER_POSITION);
				timerOuterRing_.SetScale(TIMER_SCALE_NORMAL);
				timerOuterRing_.SetInnerRadius(TIMER_GAP_OUTER_R);
				timerOuterRing_.SetOuterRadius(1.0f);
				timerOuterRing_.SetFillAmount(1.0f);
				timerOuterRing_.SetArcSpan(Math::PI2);
				timerOuterRing_.SetFillColor(TIMER_COLOR_WHITE);
				timerOuterRing_.SetEmptyColor(TIMER_COLOR_WHITE);
			}
			/** 針（動く） */
			{
				needleSprite_.Init("Assets/ui/timer/clock_hand.dds", 500.0f, 500.0f);
				needleSprite_.SetPosition(TIMER_POSITION);
				needleSprite_.SetScale(0.2f);
				needleSprite_.SetMulColor(TIMER_COLOR_RED);
				needleSprite_.SetPivot({ 0.5f, 0.25f });
			}
			/** 針（固定・12時方向） */
			{
				needleFixedSprite_.Init("Assets/ui/timer/clock_hand.dds", 500.0f, 500.0f);
				needleFixedSprite_.SetPosition(TIMER_POSITION);
				needleFixedSprite_.SetScale(0.2f);
				needleFixedSprite_.SetPivot({ 0.5f, 0.25f });
			}
		}

		TimerUIObject::~TimerUIObject()
		{}

		void TimerUIObject::Update()
		{
			if (isCounting_)
			{
				timer_ -= g_gameTime->GetFrameDeltaTime();
			}
			timer_ = max(timer_, 0.0f);

			const float ratio = (maxTime_ > 0.0f) ? (timer_ / maxTime_) : 0.0f;
			/** fillAmountを反転して時計回りに減るように */
			const float fillAmount = 1.0f - ratio;
			timerGauge_.SetFillAmount(fillAmount);
			timerBackGround_.SetFillAmount(fillAmount);

			isVisible_ = true;
			ApplyGaugeColor(TIMER_COLOR_CREAM);

			timerBackGround_.Update();
			timerGauge_.Update();
			timerGapRing_.Update();
			timerOuterRing_.Update();

			/** 針の角度を更新（経過割合 × 2π、時計回り） */
			const float elapsedRatio = 1.0f - ratio;
			Quaternion rot;
			rot.SetRotationZ(-Math::PI2 * elapsedRatio);
			needleSprite_.SetRotation(rot);
			needleSprite_.Update();
			needleFixedSprite_.Update();
		}

		void TimerUIObject::Render(RenderContext& rc)
		{
			timerBackGround_.Draw(rc);
			timerGapRing_.Draw(rc);
			if (isVisible_)
			{
				timerGauge_.Draw(rc);
			}
			timerOuterRing_.Draw(rc);
			needleSprite_.Draw(rc);
			needleFixedSprite_.Draw(rc);
		}

		void TimerUIObject::ApplyGaugeColor(const Vector4& fillColor)
		{
			timerGauge_.SetFillColor(TIMER_COLOR_EMPTY);
			timerGauge_.SetEmptyColor(fillColor);
			timerBackGround_.SetFillColor(TIMER_COLOR_EMPTY);
			timerBackGround_.SetEmptyColor(fillColor);
			timerOuterRing_.SetFillColor(TIMER_COLOR_WHITE);
			timerOuterRing_.SetEmptyColor(TIMER_COLOR_WHITE);
		}




		/**********************************************/


		PlayerHpUIObject::PlayerHpUIObject()
		{
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterHpUIParameter>("Assets/master/HpUIParameter.json", [](const nlohmann::json& j, app::core::MasterHpUIParameter& p)
				{
					/** HPバー座標X */
					char hpBarPositionX[] = "hpBarPositionXA";
					const uint32_t barPosX = ARRAYSIZE(p.hpBarPositionX);
					for (uint32_t i = 0; i < barPosX; ++i) {
						hpBarPositionX[14] = 'A' + i;
						p.hpBarPositionX[i] = j[hpBarPositionX];
					}

					/** HPバーのスケールX */
					char hpBarScaleX[] = "hpBarScaleXA";
					const uint32_t barScaleX = ARRAYSIZE(p.hpBarScaleX);
					for (uint32_t i = 0; i < barScaleX; ++i) {
						hpBarScaleX[11] = 'A' + i;
						p.hpBarScaleX[i] = j[hpBarScaleX];
					}

					/** レベルバー座標X */
					char levelBarPositionX[] = "levelBarPositionXA";
					const uint32_t PosX = ARRAYSIZE(p.levelBarPositionX);
					for (uint32_t i = 0; i < PosX; ++i) {
						levelBarPositionX[17] = 'A' + i;
						p.levelBarPositionX[i] = j[levelBarPositionX];
					}

					/** レベルバーのスケールX */
					char levelBarScaleX[] = "levelBarScaleXA";
					const uint32_t ScaleX = ARRAYSIZE(p.levelBarScaleX);
					for (uint32_t i = 0; i < ScaleX; ++i) {
						levelBarScaleX[14] = 'A' + i;
						p.levelBarScaleX[i] = j[levelBarScaleX];
					}
				});

			auto* parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterHpUIParameter>();
			/** 初期値設定 */
			index_ = MAX_LEVEL;
			damagePosX_ = 1.0f;

			layout_ = std::make_unique<app::ui::Layout>();
			layout_->Initialize <app::ui::MenuBase>("Assets/ui/layout/hpLayout.json");
			/** レベルバーの初期位置・スケールをJSONパラメータから設定 */
			{
				auto currentLevel = layout_->GetMenu()->GetUI<UIIcon>(Hash32("currentLevel"));
				if (currentLevel)
				{
					currentLevel->transform.localPosition.x = parameter->levelBarPositionX[0];
					currentLevel->transform.localScale.x = parameter->levelBarScaleX[0];
				}
			}
			/** Circleの背景 */
			{
				bgCircle_.Init(nullptr, 200.0f, 200.0f);
				bgCircle_.SetPosition({ -740, 360, 0 });
				bgCircle_.SetScale(0.55f);
				bgCircle_.SetInnerRadius(0.0f);
				bgCircle_.SetFillColor({ 0,0,0,1 });
				bgCircle_.SetEmptyColor({ 0,0,0,1 });
			}
			/** HPゲージ */
			{
				hpGauge_.Init(nullptr, 200.0f, 200.0f);
				hpGauge_.SetPosition({ -740, 360, 0 });
				/** リングの中心半径と幅を指定 */
				hpGauge_.SetInnerRadius(0.32);
				hpGauge_.SetOuterRadius(1.0);
				hpGauge_.SetScale(0.5f);
			}
			/** アイコン */
			{
				icon_.Init("Assets/ui/hp/playerIcon.DDS", 90.0f, 90.0f);
				icon_.SetPosition({ -740, 360, 0 });
				icon_.SetScale({ 1,1,1 });
			}
		}

		PlayerHpUIObject::~PlayerHpUIObject()
		{}

		void PlayerHpUIObject::Update()
		{
			if (!layout_) return;

			auto* parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterHpUIParameter>();

			auto currentHP = layout_->GetMenu()->GetUI<UIIcon>(Hash32("currentHP"));
			auto damageHP = layout_->GetMenu()->GetUI<UIIcon>(Hash32("damageHP"));
			auto currentLevel = layout_->GetMenu()->GetUI<UIIcon>(Hash32("currentLevel"));

			if (!currentHP || !damageHP || !currentLevel) return;

			/** 無敵中の点滅処理 */
			if (isInvincible_)
			{
				invincibleTimer_ -= g_gameTime->GetFrameDeltaTime();
				blinkTimer_ += g_gameTime->GetFrameDeltaTime();

				if (blinkTimer_ >= BLINK_INTERVAL)
				{
					isVisible_ = !isVisible_;
					blinkTimer_ = 0.0f;
				}

				if (invincibleTimer_ <= 0.0f)
				{
					isInvincible_ = false;
					isVisible_ = true;
				}

				/** Playerモデルを点滅 */
				if (player_)
				{
					player_->GetModelRender()->SetVisible(isVisible_);
				}
			}
			else
			{
				/** 通常時は必ず表示 */
				if (player_)
				{
					player_->GetModelRender()->SetVisible(true);
				}
			}

			/** プレイヤーのHPが有効なら反映、無効ならデバッグボタンで操作 */
			if (player_ && player_->GetStatus()->GetMaxHp() > 0.0f)
			{
				float maxHp = player_->GetStatus()->GetMaxHp();
				float curHp = player_->GetStatus()->GetCurrentHp();

				int newIndex = static_cast<int>((curHp / maxHp) * static_cast<float>(MAX_LEVEL));
				targetHpRatio_ = static_cast<float>(newIndex) / static_cast<float>(MAX_LEVEL);

				if (newIndex < index_) // HP減少
				{
					damagePosX_ = damageHP->color.x;
					damageDelayTimer_ = DAMAGE_DELAY_TIME;
					lerpVal_ = 0.0f;
				}
				index_ = newIndex;
			}
			/** NOTE: 確認でき次第削除 */
			else
			{
				// デバッグ：左ボタンでHP減少
				if (g_pad[0]->IsTrigger(enButtonLeft))
				{
					index_ = max(0, index_ - 1);
					damageDelayTimer_ = DAMAGE_DELAY_TIME;
					lerpVal_ = 0.0f;
					damagePosX_ = damageHP->color.x;
				}
				targetHpRatio_ = static_cast<float>(index_) / static_cast<float>(MAX_LEVEL);
			}

			/** 回復アニメーション中は指数関数的に補間、通常時は即時追従 */
			if (isHealAnimating_)
			{
				healAnimTimer_ += g_gameTime->GetFrameDeltaTime();
				float t = min(healAnimTimer_ / healAnimDuration_, 1.0f);

				float easedT = (t >= 1.0f) ? 1.0f : 1.0f - powf(2.0f, -10.0f * t);
				displayHpRatio_ = healStartRatio_ + (1.0f - healStartRatio_) * easedT;

				if (!healEffectFired_ && displayHpRatio_ >= 0.9f && EffectManager2D::IsAvailable())
				{
					EffectManager2D::Get().PlayEffect(
						enEffectKind2D_PlayerUIHealHP,
						{ -320.0f, 340.0f, 0.0f }
					);
					healEffectFired_ = true;
				}
				if (t >= 1.0f)
				{
					isHealAnimating_ = false;
					displayHpRatio_ = 1.0f;
				}
			}
			else
			{
				/** 通常時は即時追従 */
				displayHpRatio_ = targetHpRatio_;
			}

			/** HP割合を毎フレーム計算して設定 */
			float currentRatio = displayHpRatio_;
			currentHP->color.x = currentRatio;
			currentHP->color.y = HP_BAR_LEFT_W;
			currentHP->color.z = currentRatio;
			// color.w はアニメーションに任せるので上書きしない

			// 低HP点滅（color.wだけ操作）
			if (currentRatio <= HP_BLINK_THRESHOLD)
			{
				blinkTimer_ += g_gameTime->GetFrameDeltaTime() * 5.0f;
				currentHP->color.w = (sin(blinkTimer_) + 1.0f) * 0.5f;
			}
			else
			{
				blinkTimer_ = 0.0f;
				currentHP->color.w = 1.0f;
			}

			// ダメージバーのLerp更新（毎フレーム）
			if (lerpVal_ < 1.0f && damageDelayTimer_ < 0.0f)
			{
				lerpVal_ += 1.0f * g_gameTime->GetFrameDeltaTime();
				lerpVal_ = min(lerpVal_, 1.0f);

				float dmgRatio = (currentRatio * lerpVal_) + (damagePosX_ * (1.0f - lerpVal_));
				damageHP->color.x = dmgRatio;
				damageHP->color.y = HP_BAR_LEFT_W;
				damageHP->color.z = 0.0;
				damageHP->color.w = 1.0f;
			}
			else if (damageDelayTimer_ >= 0.0f)
			{
				damageDelayTimer_ -= g_gameTime->GetFrameDeltaTime();
				damageHP->color.x = damagePosX_;
				damageHP->color.y = HP_BAR_LEFT_W;
				damageHP->color.z = 0.0f;
				damageHP->color.w = 1.0f;
			}
			else
			{
				damageHP->color.x = currentRatio;
				damageHP->color.y = HP_BAR_LEFT_W;
				damageHP->color.z = 0.0f;
				damageHP->color.w = 1.0f;
			}

			/** ゲージが満タンになったら表示が追いつくまで待機 */
			if (levelUpIndex_ >= MAX_LEVEL && level_ < MAX_LEVEL)
			{
				if (displayLevelRatio_ >= 1.0f - 0.001f)
				{
					level_++;
					isLevelUpPending_ = true;
					if (levelUpUIObject_)
					{
						levelUpUIObject_->TriggerLevelUp(level_);
					}

					/** HP全回復フラグをセット */
					if (player_)
					{
						player_->GetStatus()->SetCurrentHp(player_->GetStatus()->GetMaxHp());
					}
					/** 現在の表示位置からスタート */
					healStartRatio_ = displayHpRatio_;
					healAnimTimer_ = 0.0f;
					isHealAnimating_ = true;
					healEffectFired_ = false;
					if (healStartRatio_ >= 0.9f && EffectManager2D::IsAvailable())
					{
						EffectManager2D::Get().PlayEffect(
							enEffectKind2D_PlayerUIHealHP,
							{ -320.0f, 340.0f, 0.0f }
						);
						healEffectFired_ = true;
					}
					index_ = MAX_LEVEL;
					targetHpRatio_ = 1.0f;
					damagePosX_ = 1.0f;
					damageDelayTimer_ = -1.0f;
					lerpVal_ = 1.0f;

					levelUpIndex_ = 0;
					displayLevelRatio_ = 0.0f;
					currentLevel->transform.localPosition.x = parameter->levelBarPositionX[0];
					currentLevel->transform.localScale.x = parameter->levelBarScaleX[0];

					// 落下演出：数字を上に配置してアニメーション開始
					{
						auto* levelDigit = layout_->GetMenu()->GetUI<app::ui::UIDigit>(Hash32("levelNumbers"));
						if (levelDigit)
						{
							levelDigit->transform.localPosition.y = 440.0f;
						}
						levelNumAnimTimer_ = 0.0f;
					}
				}
			}
			/** Lv.MAX時はゲージを満タン固定 */
			if (level_ >= MAX_LEVEL)
			{
				levelUpIndex_ = MAX_LEVEL;
				currentLevel->transform.localPosition.x = parameter->levelBarPositionX[MAX_LEVEL];
				currentLevel->transform.localScale.x = parameter->levelBarScaleX[MAX_LEVEL];
			}

			// デバッグ: 右ボタンでレベルアップ
			if (g_pad[0]->IsTrigger(enButtonRight))

			{
				if (level_ < MAX_LEVEL)
				{
					levelUpIndex_ = min(MAX_LEVEL, levelUpIndex_ + GOIN_EXP);
					currentLevel->transform.localPosition.x = parameter->levelBarPositionX[levelUpIndex_];
					currentLevel->transform.localScale.x = parameter->levelBarScaleX[levelUpIndex_];
				}
			}


			/** レベル数値の表示 */
			{
				auto levelDigit = layout_->GetMenu()->GetUI<app::ui::UIDigit>(Hash32("levelNumbers"));
				if (levelDigit)
				{
					levelDigit->SetNumber(level_);
				}
			}
			layout_->Update();

			bgCircle_.Update();

			// 経験値ゲージを線形補間で滑らかに上昇させる
			float targetLevelRatio = static_cast<float>(levelUpIndex_) / static_cast<float>(MAX_LEVEL);

			// Lerpで displayLevelRatio_ を目標値に近づける
			float dt = g_gameTime->GetFrameDeltaTime();
			displayLevelRatio_ += (targetLevelRatio - displayLevelRatio_) * levelLerpSpeed_ * dt;

			// 微小誤差でガタつかないようにスナップ
			if (std::abs(targetLevelRatio - displayLevelRatio_) < 0.001f)
			{
				displayLevelRatio_ = targetLevelRatio;
			}

			hpGauge_.SetFillAmount(displayLevelRatio_);
			hpGauge_.Update();

			icon_.Update();

			// 落下 + 着地揺れアニメーション
			// Phase1: 重力加速で落下（t^3 EaseIn）
			// Phase2: 着地後、数字 → Lv.の順に減衰振動
			if (levelNumAnimTimer_ >= 0.0f)
			{
				static constexpr float FALL_DURATION  = 0.28f;
				static constexpr float SHAKE_DURATION = 0.5f;
				static constexpr float FALL_START_Y   = 440.0f;
				static constexpr float FALL_END_Y     = 394.0f;
				static constexpr float LV_BASE_Y      = 394.0f;
				static constexpr float LV_DELAY       = 0.06f;
				static constexpr float SHAKE_FREQ     = 28.0f;
				static constexpr float SHAKE_DECAY    = 16.0f;

				levelNumAnimTimer_ += dt;

				auto* menu       = layout_->GetMenu();
				auto* levelDigit = menu->GetUI<app::ui::UIDigit>(Hash32("levelNumbers"));
				auto* lvIcon     = menu->GetUI<app::ui::UIIcon>(Hash32("levelIcon"));

				if (levelNumAnimTimer_ < FALL_DURATION)
				{
					// Phase1: 落下
					float t      = levelNumAnimTimer_ / FALL_DURATION;
					float easedT = t * t * t;
					if (levelDigit) levelDigit->transform.localPosition.y = FALL_START_Y + (FALL_END_Y - FALL_START_Y) * easedT;
					if (lvIcon)     lvIcon->transform.localPosition.y = LV_BASE_Y;
				}
				else
				{
					// Phase2: 着地揺れ（減衰正弦波）
					float shakeT = levelNumAnimTimer_ - FALL_DURATION;

					// 数字の揺れ
					float numShake = 8.0f * sinf(SHAKE_FREQ * shakeT) * expf(-SHAKE_DECAY * shakeT);
					if (levelDigit) levelDigit->transform.localPosition.y = FALL_END_Y + numShake;

					// Lv.アイコンの揺れ（衝撃が伝播：少し遅れる）
					if (shakeT >= LV_DELAY)
					{
						float lvT     = shakeT - LV_DELAY;
						float lvShake = 5.0f * sinf(SHAKE_FREQ * lvT) * expf(-SHAKE_DECAY * lvT);
						if (lvIcon) lvIcon->transform.localPosition.y = LV_BASE_Y + lvShake;
					}

					if (shakeT >= SHAKE_DURATION)
					{
						if (levelDigit) levelDigit->transform.localPosition.y = FALL_END_Y;
						if (lvIcon)     lvIcon->transform.localPosition.y = LV_BASE_Y;
						levelNumAnimTimer_ = -1.0f;
					}
				}
			}
		}

		void PlayerHpUIObject::Render(RenderContext& rc)
		{
			if (layout_) {
				layout_->Render(rc);
				bgCircle_.Draw(rc);

				hpGauge_.Draw(rc);
				icon_.Draw(rc);
			}
		}





		/**********************************************/


		EnemyHpUIObject::EnemyHpUIObject()
		{
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterEnemyHpUIParameter>("Assets/master/EnemyHpUIParameter.json", [](const nlohmann::json& j, app::core::MasterEnemyHpUIParameter& p)
				{
					// hpバー座標X
					char hpBarPositionX[] = "hpBarPositionXA";
					const uint32_t barPosX = ARRAYSIZE(p.enemyHpBarPositionX);
					for (uint32_t i = 0; i < barPosX; ++i) {
						hpBarPositionX[14] = 'A' + i;
						p.enemyHpBarPositionX[i] = j[hpBarPositionX];
					}

					// HPバーのスケールX
					char hpBarScaleX[] = "hpBarScaleXA";
					const uint32_t barScaleX = ARRAYSIZE(p.enemyHpBarScaleX);
					for (uint32_t i = 0; i < barScaleX; ++i) {
						hpBarScaleX[11] = 'A' + i;
						p.enemyHpBarScaleX[i] = j[hpBarScaleX];
					}
				});

			auto* parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterEnemyHpUIParameter>();

			layout_ = std::make_unique<app::ui::Layout>();
			layout_->Initialize <app::ui::MenuBase>("Assets/ui/layout/enemyHpLayout.json");

			// JSONのpositionをオフセットとして保存
			auto menu = layout_->GetMenu();
			auto enemyCurHP = menu->GetUI<UIIcon>(Hash32("enemyCurrentHP"));
			auto enemyDmgHP = menu->GetUI<UIIcon>(Hash32("enemyDamageHP"));

			if (enemyCurHP) curHpOffsetX_ = enemyCurHP->transform.localPosition.x;
			if (enemyDmgHP) dmgHpOffsetX_ = enemyDmgHP->transform.localPosition.x;

			hpIndex_ = MAX_LEVEL;
			damagePosX_ = 1.0f;
		}

		EnemyHpUIObject::~EnemyHpUIObject()
		{}

		void EnemyHpUIObject::Update()
		{
			// 削除予約済みなら何もしない
			if (isDead_) return;
			if (!player_) return;

			Vector3 worldPos = Vector3::Zero;

			if (stoneTarget_)
			{
				worldPos = stoneTarget_->transform.position;
			}
			else if (mushroomTarget_)
			{
				worldPos = mushroomTarget_->transform.position;
			}
			else
			{
				return;
			}

			/** 距離チェック */
			Vector3 diff = worldPos - player_->transform.position;
			float distance = diff.Length();

			// 距離がしきい値以内なら表示
			isVisible_ = (distance < DRAW_DISTANCE);

			// 非表示なら更新処理もスキップして負荷を減らす
			if (!isVisible_) return;

			worldPos.y += 70.0f;

			// ワールド座標からスクリーン座標に変換
			Vector2 screenPos;
			g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, worldPos);

			// layoutの位置を更新
			auto menu = layout_->GetMenu();
			if (menu)
			{
				auto enemyBg = menu->GetUI<UIIcon>(Hash32("enemyBackground"));
				auto enemyBgHP = menu->GetUI<UIIcon>(Hash32("enemyBackGroundHP"));
				auto enemyDmgHP = menu->GetUI<UIIcon>(Hash32("enemyDamageHP"));
				auto enemyCurHP = menu->GetUI<UIIcon>(Hash32("enemyCurrentHP"));

				// 全要素を同じスクリーン座標に移動
				auto moveToScreen = [&](UIIcon* ui) {
					if (ui) {
						ui->transform.localPosition.x = screenPos.x;
						ui->transform.localPosition.y = screenPos.y;
					}
					};
				moveToScreen(enemyBg);
				moveToScreen(enemyBgHP);

				auto* parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterEnemyHpUIParameter>();

				{
					int currentHP = 0;
					int maxHP = 1;

					if (stoneTarget_)
					{
						currentHP = stoneTarget_->GetCurrentHP();
						auto* status = stoneTarget_->GetStatus();
						if (status == nullptr)
						{
							// statusが無効な場合は安全に終了フラグを立てて次フレームで破棄
							isDead_ = true;
							return;
						}
						maxHP = static_cast<int>(stoneTarget_->GetStatus()->GetMaxHp());
					}
					else if (mushroomTarget_)
					{
						currentHP = mushroomTarget_->GetCurrentHP();
						auto* status = mushroomTarget_->GetStatus();
						if (status == nullptr)
						{
							// statusが無効な場合は安全に終了フラグを立てて次フレームで破棄
							isDead_ = true;
							return;
						}
						maxHP = static_cast<int>(mushroomTarget_->GetStatus()->GetMaxHp());
					}

					// HP割合から index を計算（0〜MAX_LEVEL）
					float ratio = static_cast<float>(currentHP) / static_cast<float>(maxHP);
					int newIndex = static_cast<int>(ratio * MAX_LEVEL);
					newIndex = max(0, min(MAX_LEVEL, newIndex));

					// HPが減った瞬間を検知 → ディレイ開始
					if (newIndex < hpIndex_)
					{
						damageDelayTimer_ = 0.3f;
						lerpVal_ = 0.0f;
						damagePosX_ = enemyDmgHP->color.x; // 現在の表示割合を保存
					}
					hpIndex_ = newIndex;
				}
				// 即時反映
				if (g_pad[0]->IsTrigger(enButtonUp))
				{
					damageDelayTimer_ = 0.3f;
					lerpVal_ = 0.0f;
					damagePosX_ = enemyDmgHP->color.x;
					hpIndex_ = max(0, hpIndex_ - 1);
				}
				// 即時反映
				if (g_pad[0]->IsTrigger(enButtonDown))
				{
					hpIndex_ = min(MAX_LEVEL, hpIndex_ + 1);
				}

				float currentRatio = static_cast<float>(hpIndex_) / static_cast<float>(MAX_LEVEL);

				// currentHP即時反映
				enemyCurHP->color.x = currentRatio;
				enemyCurHP->color.y = HP_BAR_LEFT_W;
				enemyCurHP->color.z = currentRatio;
				enemyCurHP->color.w = 1.0f;
				enemyCurHP->transform.localPosition.x = screenPos.x + 5.0f;
				enemyCurHP->transform.localPosition.y = screenPos.y;

				// damageHPのLerp
				if (lerpVal_ < 1.0f && damageDelayTimer_ < 0.0f)
				{
					lerpVal_ += 1.0f * g_gameTime->GetFrameDeltaTime();
					lerpVal_ = min(lerpVal_, 1.0f);
					float dmgRatio = (currentRatio * lerpVal_) + (damagePosX_ * (1.0f - lerpVal_));
					enemyDmgHP->color.x = dmgRatio;
					enemyDmgHP->color.y = HP_BAR_LEFT_W;
					enemyDmgHP->color.z = 0.0f;
					enemyDmgHP->color.w = 1.0f;
				}
				else if (damageDelayTimer_ >= 0.0f)
				{
					damageDelayTimer_ -= g_gameTime->GetFrameDeltaTime();
					enemyDmgHP->color.x = damagePosX_;
					enemyDmgHP->color.y = HP_BAR_LEFT_W;
					enemyDmgHP->color.z = 0.0f;
					enemyDmgHP->color.w = 1.0f;
				}
				else
				{
					enemyDmgHP->color.x = currentRatio;
					enemyDmgHP->color.y = HP_BAR_LEFT_W;
					enemyDmgHP->color.z = 0.0f;
					enemyDmgHP->color.w = 1.0f;
				}
				enemyDmgHP->transform.localPosition.x = screenPos.x + 5.0f;
				enemyDmgHP->transform.localPosition.y = screenPos.y;

				// アイコンの切り替えと追従 
				auto stoneIcon = menu->GetUI<UIIcon>(Hash32("stoneIcon"));
				auto mushroomIcon = menu->GetUI<UIIcon>(Hash32("mushroomIcon"));

				// アイコンをHPバーの左に配置するためのオフセット値
				const float ICON_OFFSET_X = -65.0f;
				const float ICON_OFFSET_Y = 10.0f;

				if (stoneTarget_)
				{
					// ストーン用アイコンを表示して追従させる
					if (stoneIcon) {
						stoneIcon->transform.localPosition.x = screenPos.x + ICON_OFFSET_X;
						stoneIcon->transform.localPosition.y = screenPos.y + ICON_OFFSET_Y;
						stoneIcon->transform.localScale = Vector3::One;
					}
					// マッシュルーム用アイコンは不要なのでスケール0で非表示
					if (mushroomIcon) {
						mushroomIcon->transform.localScale = Vector3::Zero;
					}
				}
				else if (mushroomTarget_)
				{
					// マッシュルーム用アイコンを表示して追従させる
					if (mushroomIcon) {
						mushroomIcon->transform.localPosition.x = screenPos.x + ICON_OFFSET_X;
						mushroomIcon->transform.localPosition.y = screenPos.y + ICON_OFFSET_Y;
						mushroomIcon->transform.localScale = Vector3::One;
					}
					// ストーン用アイコンは不要なのでスケール0で非表示
					if (stoneIcon) {
						stoneIcon->transform.localScale = Vector3::Zero;
					}
				}
				layout_->Update();
			}
		}

		void EnemyHpUIObject::Render(RenderContext& rc)
		{
			if (layout_ && isVisible_) {
				layout_->Render(rc);
			}
		}




		/**********************************************/


		LevelUpUIObject::LevelUpUIObject()
		{
			layout_ = std::make_unique<app::ui::Layout>();
			layout_->Initialize <app::ui::MenuBase>("Assets/ui/layout/LevelUpNotifierLayout.json");
		}

		LevelUpUIObject::~LevelUpUIObject()
		{}

		void LevelUpUIObject::Update()
		{
			if (!layout_) return;

			if (!isLevelUpPending_)
			{
				layout_->Update();
				return;
			}

			auto* menu = layout_->GetMenu();
			if (menu)
			{
				PlayEntryAnimation(menu);

				// 登場アニメーション完了後に退場アニメーションへ移行
				if (isAtkAnimPlayed_ && isLevelAnimPlayed_)
				{
					PlayExitAnimation(menu);
				}
			}

			layout_->Update();
		}

		void LevelUpUIObject::PlayEntryAnimation(app::ui::MenuBase* menu)
		{
			// アニメーションをアタッチして再生するラムダ
			auto playSlideAnim = [](app::ui::UIIcon* icon, uint32_t animKey)
				{
					if (!icon) return;
					// 常に Remove → Attach で再生成してからPlay
					icon->RemoveAnimation(animKey);
					app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(icon, animKey);
					auto* anim = icon->FindAnimation(animKey);
					if (anim) anim->Play();
				};

			auto playColorAnim = [](app::ui::UIIcon* icon, uint32_t animKey)
				{
					if (!icon) return;
					icon->RemoveAnimation(animKey);
					app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(icon, animKey);
					auto* anim = icon->FindAnimation(animKey);
					if (anim) anim->Play();
				};

			// レベルUPバー：即座に再生
			if (!isLevelAnimPlayed_)
			{
				playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarBlue")), Hash32("LevelUp_SlideY"));
				playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarBlue")), Hash32("LevelUp_SlideY"));
				playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpDefault")), Hash32("LevelUp_SlideY"));
				playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpBloom")), Hash32("LevelUp_SlideY"));
				playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarBlue")), Hash32("LevelUp_FadeIn"));
				playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarBlue")), Hash32("LevelUp_FadeIn"));
				playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpDefault")), Hash32("LevelUp_FadeIn"));
				playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpBloom")), Hash32("LevelUp_FadeIn"));

				app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::LevelUp), false);
				isLevelAnimPlayed_ = true;
			}

			// 攻撃力UPバー：0.3秒遅れて再生
			if (!isAtkAnimPlayed_)
			{
				if (!isAtkUpLevel_)
				{
					// 攻撃力UPが不要なレベルはスキップ済み扱い
					isAtkAnimPlayed_ = true;
					return;
				}

				levelAnimTimer_ += g_gameTime->GetFrameDeltaTime();
				if (levelAnimTimer_ >= 0.3f)
				{
					playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarOrange")), Hash32("AtkUp_SlideY"));
					playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarOrange")), Hash32("AtkUp_SlideY"));
					playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpDefault")), Hash32("AtkUp_SlideY"));
					playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpBloom")), Hash32("AtkUp_SlideY"));
					playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarOrange")), Hash32("AtkUp_FadeIn"));
					playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarOrange")), Hash32("AtkUp_FadeIn"));
					playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpDefault")), Hash32("AtkUp_FadeIn"));
					playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpBloom")), Hash32("AtkUp_FadeIn"));
					isAtkAnimPlayed_ = true;
				}
			}
		}

		void LevelUpUIObject::PlayExitAnimation(app::ui::MenuBase* menu)
		{
			auto playSlideAnim = [](app::ui::UIIcon* icon, uint32_t animKey)
				{
					if (!icon) return;
					icon->RemoveAnimation(animKey);
					app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(icon, animKey);
					auto* anim = icon->FindAnimation(animKey);
					if (anim) anim->Play();
				};

			auto playColorAnim = [](app::ui::UIIcon* icon, uint32_t animKey)
				{
					if (!icon) return;
					icon->RemoveAnimation(animKey);
					app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(icon, animKey);
					auto* anim = icon->FindAnimation(animKey);
					if (anim) anim->Play();
				};

			exitAnimTimer_ += g_gameTime->GetFrameDeltaTime();

			// 2.0秒後：左スライド + フェードアウトを同時再生
			if (!isExitLeftPlayed_ && exitAnimTimer_ >= 2.0f)
			{
				playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarBlue")),      Hash32("LevelUp_ExitX_Left"));
				playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarBlue")),      Hash32("LevelUp_ExitX_Left"));
				playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpDefault")),    Hash32("LevelUp_ExitX_Left"));
				playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpBloom")),      Hash32("LevelUp_ExitX_Left"));
				playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarBlue")),      Hash32("LevelUp_ExitFadeOut"));
				playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarBlue")),      Hash32("LevelUp_ExitFadeOut"));
				playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpDefault")),    Hash32("LevelUp_ExitFadeOut"));
				playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpBloom")),      Hash32("LevelUp_ExitFadeOut"));

				if (isAtkUpLevel_)
				{
					playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarOrange")),    Hash32("AtkUp_ExitX_Left"));
					playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarOrange")),    Hash32("AtkUp_ExitX_Left"));
					playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpDefault")), Hash32("AtkUp_ExitX_Left"));
					playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpBloom")),   Hash32("AtkUp_ExitX_Left"));
					playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarOrange")),    Hash32("AtkUp_ExitFadeOut"));
					playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarOrange")),    Hash32("AtkUp_ExitFadeOut"));
					playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpDefault")), Hash32("AtkUp_ExitFadeOut"));
					playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpBloom")),   Hash32("AtkUp_ExitFadeOut"));
				}

				isExitLeftPlayed_ = true;
				exitLeftTimer_ = 0.0f;
			}

			// 退場完了後にリセットへ移行
			if (isExitLeftPlayed_)
			{
				exitLeftTimer_ += g_gameTime->GetFrameDeltaTime();
				if (exitLeftTimer_ >= 1.25f)
				{
					ResetAnimation(menu);
				}
			}
		}

		void LevelUpUIObject::ResetAnimation(app::ui::MenuBase* menu)
		{
			// 全アニメーションを削除するラムダ
			auto clearAnims = [](app::ui::UIIcon* icon)
				{
					if (!icon) return;
					icon->RemoveAnimation(Hash32("LevelUp_SlideY"));
					icon->RemoveAnimation(Hash32("LevelUp_ExitX_Left"));
					icon->RemoveAnimation(Hash32("LevelUp_ExitFadeOut"));
					icon->RemoveAnimation(Hash32("LevelUp_FadeIn"));
					icon->RemoveAnimation(Hash32("AtkUp_SlideY"));
					icon->RemoveAnimation(Hash32("AtkUp_ExitX_Left"));
					icon->RemoveAnimation(Hash32("AtkUp_ExitFadeOut"));
					icon->RemoveAnimation(Hash32("AtkUp_FadeIn"));
				};

			// 座標をリセットするラムダ
			auto resetPosition = [](app::ui::UIIcon* icon, float x, float y)
				{
					if (!icon) return;
					icon->transform.localPosition.x = x;
					icon->transform.localPosition.y = y;
				};

			// 透明度をリセットするラムダ
			auto resetAlpha = [](app::ui::UIIcon* icon)
				{
					if (!icon) return;
					icon->color.w = 0.0f;
				};

			/** レベルアップ用UI */
			clearAnims(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarBlue")));
			clearAnims(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarBlue")));
			clearAnims(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpDefault")));
			clearAnims(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpBloom")));
			resetPosition(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarBlue")), -550.0f, -1260.0f);
			resetPosition(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarBlue")), -550.0f, -1260.0f);
			resetPosition(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpDefault")), -550.0f, -1260.0f);
			resetPosition(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpBloom")), -550.0f, -1260.0f);
			resetAlpha(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarBlue")));
			resetAlpha(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarBlue")));
			resetAlpha(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpDefault")));
			resetAlpha(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpBloom")));

			/** 攻撃力UP用UI */
			clearAnims(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarOrange")));
			clearAnims(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarOrange")));
			clearAnims(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpDefault")));
			clearAnims(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpBloom")));
			resetPosition(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarOrange")), -550.0f, -1180.0f);
			resetPosition(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarOrange")), -550.0f, -1180.0f);
			resetPosition(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpDefault")), -550.0f, -1180.0f);
			resetPosition(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpBloom")), -550.0f, -1180.0f);
			resetAlpha(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarOrange")));
			resetAlpha(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarOrange")));
			resetAlpha(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpDefault")));
			resetAlpha(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpBloom")));

			// 演出完了フラグをリセット
			isLevelUpPending_ = false;
		}

		void LevelUpUIObject::Render(RenderContext& rc)
		{
			if (layout_) {
				layout_->Render(rc);
			}
		}
	}
}
