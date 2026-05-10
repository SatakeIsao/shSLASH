#include "stdafx.h"
#include "InGameUI.h"
#include "core/ParameterManager.h"
#include "actor/ActorStatus.h" 
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"

namespace {
	static app::ui::UIAnimationSequence* seq = nullptr;

	static constexpr int MAX_TIME = 10;
	static constexpr int MAX_LEVEL = 10;
	/** 獲得した経験値 */
	// 一旦、すぐレベルアップする感じで。Lv.10に近づくにつれて、もらう経験値少なくしたい
	static constexpr int GOIN_EXP = 1;

	static constexpr float DRAW_DISTANCE = 400.0f;
	static constexpr float DAMAGE_DELAY_TIME = 0.5f;

	// HPバーシェーダー用
	// テクスチャの斜め部分の幅をUV空間で指定する
	static constexpr float HP_BAR_LEFT_W = 0.06f;  // 左は斜めなし
	static constexpr float HP_BAR_RIGHT_W = 0.0f;  // 右斜めの幅 (実測して調整)

	/** 無敵時間 */
	static constexpr float BLINK_INTERVAL = 0.01f;
}

namespace app
{
	namespace ui
	{
		TimerUIObject::TimerUIObject()
		{
			layout_ = std::make_unique<app::ui::Layout>();
			layout_->Initialize <app::ui::MenuBase>("Assets/ui/layout/timerLayout.json");

			timer_ = MAX_TIME;
		}

		TimerUIObject::~TimerUIObject()
		{
		}

		void TimerUIObject::Update()
		{
			if (timer_ <= 0.0f) {
				timer_ = 0.0f;
			}

			if (!layout_) return; // layout自体がなければ何もしない

			auto menu = layout_->GetMenu();
			if (menu)
			{
				// 取得に失敗（nullptr）した場合の安全策を強化
				auto timerDigit = menu->GetUI<app::ui::UIDigit>(Hash32("timerNumbers"));
				if (timerDigit)
				{
					timerDigit->SetZeroPadding(true);
					timerDigit->SetNumber(static_cast<int>(std::ceil(timer_)));
				}
			}
			layout_->Update();
		}

		void TimerUIObject::Render(RenderContext& rc)
		{
			if (layout_) {
				layout_->Render(rc);
			}
		}




		/**********************************************/


		PlayerHpUIObject::PlayerHpUIObject()
		{
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterHpUIParameter>("Assets/master/HpUIParameter.json", [](const nlohmann::json& j, app::core::MasterHpUIParameter& p)
				{
					// hpバー座標X
					char hpBarPositionX[] = "hpBarPositionXA";
					const uint32_t barPosX = ARRAYSIZE(p.hpBarPositionX);
					for (uint32_t i = 0; i < barPosX; ++i) {
						hpBarPositionX[14] = 'A' + i;
						p.hpBarPositionX[i] = j[hpBarPositionX];
					}

					// HPバーのスケールX
					char hpBarScaleX[] = "hpBarScaleXA";
					const uint32_t barScaleX = ARRAYSIZE(p.hpBarScaleX);
					for (uint32_t i = 0; i < barScaleX; ++i) {
						hpBarScaleX[11] = 'A' + i;
						p.hpBarScaleX[i] = j[hpBarScaleX];
					}

					// レベルバー座標X
					char levelBarPositionX[] = "levelBarPositionXA";
					const uint32_t PosX = ARRAYSIZE(p.levelBarPositionX);
					for (uint32_t i = 0; i < PosX; ++i) {
						levelBarPositionX[17] = 'A' + i;
						p.levelBarPositionX[i] = j[levelBarPositionX];
					}

					// レベルバーのスケールX
					char levelBarScaleX[] = "levelBarScaleXA";
					const uint32_t ScaleX = ARRAYSIZE(p.levelBarScaleX);
					for (uint32_t i = 0; i < ScaleX; ++i) {
						levelBarScaleX[14] = 'A' + i;
						p.levelBarScaleX[i] = j[levelBarScaleX];
					}
				});

			auto* parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterHpUIParameter>();

			index_ = MAX_LEVEL;
			damagePosX_ = 1.0f;

			layout_ = std::make_unique<app::ui::Layout>();
			layout_->Initialize <app::ui::MenuBase>("Assets/ui/layout/hpLayout.json");

			auto currentLevel = layout_->GetMenu()->GetUI<UIIcon>(Hash32("currentLevel"));
			if (currentLevel)
			{
				currentLevel->transform.localPosition.x = parameter->levelBarPositionX[0];
				currentLevel->transform.localScale.x = parameter->levelBarScaleX[0];
			}

			index_ = MAX_LEVEL;
			damagePosX_ = 1.0f;

			// Circleの背景
			{
				bgCircle_.Init(nullptr, 200.0f, 200.0f);
				bgCircle_.SetPosition({ -740, 360, 0 });
				bgCircle_.SetScale(0.55f);
				bgCircle_.SetInnerRadius(0.0f);        // 穴なし（塗りつぶし円）
				bgCircle_.SetFillColor({ 0,0,0,1 });   // 黒
				bgCircle_.SetEmptyColor({ 0,0,0,1 });  // 黒（空エリアも黒）
			}
			// HPゲージ
			{
				hpGauge_.Init(nullptr, 200.0f, 200.0f);
				hpGauge_.SetPosition({ -740, 360, 0 });
				// リングの中心半径と幅を指定する例
				hpGauge_.SetInnerRadius(0.32);
				hpGauge_.SetOuterRadius(1.0);
				hpGauge_.SetScale(0.5f);
			}
			// アイコン
			{
				icon_.Init("Assets/ui/hp/playerIcon.DDS", 90.0f, 90.0f);
				icon_.SetPosition({ -740, 360, 0 });
				icon_.SetScale({ 1,1,1 });
			}
		}

		PlayerHpUIObject::~PlayerHpUIObject()
		{
		}

		void PlayerHpUIObject::Update()
		{
			if (!layout_) return;

			auto* parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterHpUIParameter>();

			auto currentHP = layout_->GetMenu()->GetUI<UIIcon>(Hash32("currentHP"));
			auto damageHP = layout_->GetMenu()->GetUI<UIIcon>(Hash32("damageHP"));
			auto currentLevel = layout_->GetMenu()->GetUI<UIIcon>(Hash32("currentLevel"));

			if (!currentHP || !damageHP || !currentLevel) return;

			// 無敵中の点滅処理
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
					isVisible_ = true; // 無敵終了時は必ず表示
				}

				// BattleCharacterのモデルを点滅
				if (player_)
				{
					player_->GetModelRender()->SetVisible(isVisible_);
				}
			}
			else
			{
				// 通常時は必ず表示
				if (player_)
				{
					player_->GetModelRender()->SetVisible(true);
				}
			}

			// プレイヤーのHPが有効なら反映、無効ならデバッグボタンで操作
			if (player_ && player_->GetStatus()->GetMaxHp() > 0.0f)
			{
				float maxHp = player_->GetStatus()->GetMaxHp();
				float curHp = player_->GetStatus()->GetCurrentHp();

				int newIndex = static_cast<int>((curHp / maxHp) * static_cast<float>(MAX_LEVEL));
				if (newIndex < index_) // HPが減った瞬間だけディレイバーを更新
				{
					damagePosX_ = damageHP->color.x;
					damageDelayTimer_ = DAMAGE_DELAY_TIME;
					lerpVal_ = 0.0f;
				}
				index_ = newIndex;
			}
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
			}

			// HP割合を毎フレーム計算してセット
			float currentRatio = static_cast<float>(index_) / static_cast<float>(MAX_LEVEL);
			currentHP->color.x = currentRatio;
			currentHP->color.y = HP_BAR_LEFT_W;
			currentHP->color.z = currentRatio;
			// color.w はアニメーションに任せるので上書きしない

			// 低HP点滅（color.wだけ操作）
			const float BLINK_THRESHOLD = 0.3f;
			if (currentRatio <= BLINK_THRESHOLD)
			{
				blinkTimer_ += g_gameTime->GetFrameDeltaTime() * 5.0f;
				currentHP->color.w = (sin(blinkTimer_) + 1.0f) * 0.5f;
			}
			else
			{
				blinkTimer_ = 0.0f;
				currentHP->color.w = 1.0f;
			}

			char buf[128];
			sprintf_s(buf, "currentRatio: %f\n", currentRatio);
			OutputDebugStringA(buf);

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
					levelUpIndex_ = 0;
					displayLevelRatio_ = 0.0f;
					currentLevel->transform.localPosition.x = parameter->levelBarPositionX[0];
					currentLevel->transform.localScale.x = parameter->levelBarScaleX[0];
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
		{
		}

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
				DeleteGO(this);
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
						if (status == nullptr) { DeleteGO(this); return; }
						maxHP = static_cast<int>(stoneTarget_->GetStatus()->GetMaxHp());
					}
					else if (mushroomTarget_)
					{
						currentHP = mushroomTarget_->GetCurrentHP();
						auto* status = mushroomTarget_->GetStatus();
						if (status == nullptr) { DeleteGO(this); return; }
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
					enemyDmgHP->color.z = dmgRatio;
					enemyDmgHP->color.w = 1.0f;
				}
				else if (damageDelayTimer_ >= 0.0f)
				{
					damageDelayTimer_ -= g_gameTime->GetFrameDeltaTime();
					enemyDmgHP->color.x = damagePosX_;
					enemyDmgHP->color.y = HP_BAR_LEFT_W;
					enemyDmgHP->color.z = damagePosX_;
					enemyDmgHP->color.w = 1.0f;
				}
				else
				{
					enemyDmgHP->color.x = currentRatio;
					enemyDmgHP->color.y = HP_BAR_LEFT_W;
					enemyDmgHP->color.z = currentRatio;
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
		{
		}

		void LevelUpUIObject::Update()
		{
			if (!layout_) return;

			// アニメーションをアタッチして再生するラムダ
			auto playSlideAnim = [](app::ui::UIIcon* icon, uint32_t animKey)
				{
					if (!icon) return;
					// 常に Remove → Attach で再生成してからPlay
					icon->RemoveAnimation(animKey);
					app::ui::UIAnimationFactory::Attach<app::ui::UITranslateAniamtion>(icon, animKey);

					auto* anim = icon->FindAnimation(animKey);
					if (anim)
					{
						anim->Play();
					}
				};

			// カラーアニメをアタッチして再生するラムダ
			auto playColorAnim = [](app::ui::UIIcon* icon, uint32_t animKey)
				{
					if (!icon) return;
					icon->RemoveAnimation(animKey);
					app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(icon, animKey);
					auto* anim = icon->FindAnimation(animKey);
					if (anim) anim->Play();
				};

			if (isLevelUpPending_)
			{
				auto* menu = layout_->GetMenu();
				if (menu)
				{
					// レベルUPバー：即座に再生
					if (!isLevelAnimPlayed_)
					{
						playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarBlue")), Hash32("LevelUp_SlideY"));
						playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarBlue")), Hash32("LevelUp_SlideY"));
						playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpDefault")), Hash32("LevelUp_SlideY"));
						playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpBloom")), Hash32("LevelUp_SlideY"));

						// フェードインも同時再生
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
							// 偶数レベル以外はスキップ
							isLevelAnimPlayed_ = true;
						}
						else
						{
							levelAnimTimer_ += g_gameTime->GetFrameDeltaTime();
							if (levelAnimTimer_ >= 0.3f)
							{
								playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarOrange")), Hash32("AtkUp_SlideY"));
								playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarOrange")), Hash32("AtkUp_SlideY"));
								playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpDefault")), Hash32("AtkUp_SlideY"));
								playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpBloom")), Hash32("AtkUp_SlideY"));

								// フェードインも同時再生
								playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarOrange")), Hash32("AtkUp_FadeIn"));
								playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarOrange")), Hash32("AtkUp_FadeIn"));
								playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpDefault")), Hash32("AtkUp_FadeIn"));
								playColorAnim(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpBloom")), Hash32("AtkUp_FadeIn"));
								isAtkAnimPlayed_ = true;
							}
						}
					}

					// 両方再生済みなら退場タイマー加算
					if (isAtkAnimPlayed_ && isLevelAnimPlayed_)
					{
						exitAnimTimer_ += g_gameTime->GetFrameDeltaTime();

						// 1.0秒後：右へ少し移動
						if (!isExitRightPlayed_ && exitAnimTimer_ >= 2.0f)
						{
							playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarOrange")), Hash32("AtkUp_ExitX_Right"));
							playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarOrange")), Hash32("AtkUp_ExitX_Right"));
							playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpDefault")), Hash32("AtkUp_ExitX_Right"));
							playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpBloom")), Hash32("AtkUp_ExitX_Right"));
							playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarBlue")), Hash32("LevelUp_ExitX_Right"));
							playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarBlue")), Hash32("LevelUp_ExitX_Right"));
							playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpDefault")), Hash32("LevelUp_ExitX_Right"));
							playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpBloom")), Hash32("LevelUp_ExitX_Right"));

							app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Slide), false);
							isExitRightPlayed_ = true;
						}

						// 1.15秒後：左へ画面外に吹っ飛ぶ
						if (isExitRightPlayed_ && !isExitLeftPlayed_ && exitAnimTimer_ >= 2.15f)
						{
							playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarOrange")), Hash32("AtkUp_ExitX_Left"));
							playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarOrange")), Hash32("AtkUp_ExitX_Left"));
							playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpDefault")), Hash32("AtkUp_ExitX_Left"));
							playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpBloom")), Hash32("AtkUp_ExitX_Left"));
							playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarBlue")), Hash32("LevelUp_ExitX_Left"));
							playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarBlue")), Hash32("LevelUp_ExitX_Left"));
							playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpDefault")), Hash32("LevelUp_ExitX_Left"));
							playSlideAnim(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpBloom")), Hash32("LevelUp_ExitX_Left"));
							isExitLeftPlayed_ = true;
							exitLeftTimer_ = 0.0f;
						}

						// 退場完了
						if (isExitLeftPlayed_)
						{
							exitLeftTimer_ += g_gameTime->GetFrameDeltaTime();

							if (exitLeftTimer_ >= 0.3f)
							{
								// アニメーションを全削除してから座標リセット
								auto clearAnims = [](app::ui::UIIcon* icon)
									{
										if (!icon) return;
										icon->RemoveAnimation(Hash32("LevelUp_SlideY"));
										icon->RemoveAnimation(Hash32("LevelUp_ExitX_Right"));
										icon->RemoveAnimation(Hash32("LevelUp_ExitX_Left"));
										icon->RemoveAnimation(Hash32("AtkUp_SlideY"));
										icon->RemoveAnimation(Hash32("AtkUp_ExitX_Right"));
										icon->RemoveAnimation(Hash32("AtkUp_ExitX_Left"));
										icon->RemoveAnimation(Hash32("LevelUp_FadeIn"));
										icon->RemoveAnimation(Hash32("AtkUp_FadeIn"));
									};

								clearAnims(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarBlue")));
								clearAnims(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarBlue")));
								clearAnims(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpDefault")));
								clearAnims(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpBloom")));
								clearAnims(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarOrange")));
								clearAnims(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarOrange")));
								clearAnims(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpDefault")));
								clearAnims(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpBloom")));

								auto resetPosition = [](app::ui::UIIcon* icon, float x, float y)
									{
										if (!icon) return;
										icon->transform.localPosition.x = x;
										icon->transform.localPosition.y = y;
									};

								auto resetAlpha = [](app::ui::UIIcon* icon)
									{
										if (!icon) return;
										icon->color.w = 0.0f;
									};

								resetPosition(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarBlue")), -550.0f, -1260.0f);
								resetPosition(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarBlue")), -550.0f, -1260.0f);
								resetPosition(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpDefault")), -550.0f, -1260.0f);
								resetPosition(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpBloom")), -550.0f, -1260.0f);
								resetPosition(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarOrange")), -550.0f, -1180.0f);
								resetPosition(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarOrange")), -550.0f, -1180.0f);
								resetPosition(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpDefault")), -550.0f, -1180.0f);
								resetPosition(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpBloom")), -550.0f, -1180.0f);

								resetAlpha(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarBlue")));
								resetAlpha(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarBlue")));
								resetAlpha(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpDefault")));
								resetAlpha(menu->GetUI<app::ui::UIIcon>(Hash32("levelUpBloom")));
								resetAlpha(menu->GetUI<app::ui::UIIcon>(Hash32("outerBarOrange")));
								resetAlpha(menu->GetUI<app::ui::UIIcon>(Hash32("innerBarOrange")));
								resetAlpha(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpDefault")));
								resetAlpha(menu->GetUI<app::ui::UIIcon>(Hash32("atkPowerUpBloom")));

								isLevelUpPending_ = false;
							}
						}
					}
				}
			}

			layout_->Update();
		}

		void LevelUpUIObject::Render(RenderContext& rc)
		{
			if (layout_) {
				layout_->Render(rc);
			}
		}
	}
}