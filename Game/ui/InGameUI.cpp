#include "stdafx.h"
#include "InGameUI.h"
#include "core/ParameterManager.h"
#include "actor/ActorStatus.h" 

namespace {
	static constexpr int MAX_TIME = 10;
	static constexpr int MAX_LEVEL = 10;

	static constexpr float DRAW_DISTANCE = 400.0f;
	static constexpr float DAMAGE_DELAY_TIME = 0.5f;
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

			if (parameter)
			{
				damagePosX_ = parameter->hpBarPositionX[0];
				damageScaleX_ = parameter->hpBarScaleX[0];
			}

			layout_ = std::make_unique<app::ui::Layout>();
			layout_->Initialize <app::ui::MenuBase>("Assets/ui/layout/hpLayout.json");

			auto currentLevel = layout_->GetMenu()->GetUI<UIIcon>(Hash32("currentLevel"));
			if (currentLevel)
			{
				currentLevel->transform.localPosition.x = parameter->levelBarPositionX[0];
				currentLevel->transform.localScale.x = parameter->levelBarScaleX[0];
			}
		}

		PlayerHpUIObject::~PlayerHpUIObject()
		{
		}

		void PlayerHpUIObject::Update()
		{
			if (!layout_) return;

			auto* parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterHpUIParameter>();

			/** HPバー座標X */
			{
				auto currentHP = layout_->GetMenu()->GetUI<UIIcon>(Hash32("currentHP"));
				auto damageHP = layout_->GetMenu()->GetUI<UIIcon>(Hash32("damageHP"));
				auto currentLevel = layout_->GetMenu()->GetUI<UIIcon>(Hash32("currentLevel"));

				if (!currentHP || !damageHP || !currentLevel) return;

				// 即時反映
				if (g_pad[0]->IsTrigger(enButtonLeft))
				{
					index_ = max(0, index_ - 1);
					// タイマーリセット
					damageDelayTimer_ = DAMAGE_DELAY_TIME;
					lerpVal_ = 0.0f;
					// 現在地を保存
					damagePosX_ = damageHP->transform.localPosition.x;
					damageScaleX_ = damageHP->transform.localScale.x;

					currentHP->transform.localPosition.x = parameter->hpBarPositionX[index_];
					currentHP->transform.localScale.x = parameter->hpBarScaleX[index_];
				}

				// levelUpIndex_ が MAX_LEVEL に達したら折り返してレベルアップ
				if (levelUpIndex_ >= MAX_LEVEL)
				{
					if (level_ < MAX_LEVEL)
					{
						level_++;
						isLevelUpPending_ = true;
					}
						levelUpIndex_ = 0; // ゲージを0に戻す

						// ゲージのUIを0の位置に即時反映
						currentLevel->transform.localPosition.x = parameter->levelBarPositionX[0];
						currentLevel->transform.localScale.x = parameter->levelBarScaleX[0];
				}
				if (level_ >= MAX_LEVEL)
				{
					// Lv.10になった瞬間にゲージをMAXに固定
					levelUpIndex_ = MAX_LEVEL;
					currentLevel->transform.localPosition.x = parameter->levelBarPositionX[MAX_LEVEL];
					currentLevel->transform.localScale.x = parameter->levelBarScaleX[MAX_LEVEL];
				}
				

				/** デバッグテスト： 右ボタン */
				if (g_pad[0]->IsTrigger(enButtonRight))
				{
					if (level_ < MAX_LEVEL)
					{
						levelUpIndex_ = min(MAX_LEVEL, levelUpIndex_ + 1); // ← ++は1回だけ
						currentLevel->transform.localPosition.x = parameter->levelBarPositionX[levelUpIndex_];
						currentLevel->transform.localScale.x = parameter->levelBarScaleX[levelUpIndex_];
					}
				}

				// ディレイとLerp更新
				if (lerpVal_ < 1.0f && damageDelayTimer_ < 0.0f)
				{
					lerpVal_ += 1.0f * g_gameTime->GetFrameDeltaTime();
					lerpVal_ = min(lerpVal_, 1.0f);

					const float targetPosX = parameter->hpBarPositionX[index_];
					const float targetScaleX = parameter->hpBarScaleX[index_];

					// 開始地点と目標地点を補間
					float currentPosX = (targetPosX * lerpVal_) + (damagePosX_ * (1.0f - lerpVal_));
					float currentScaleX = (targetScaleX * lerpVal_) + (damageScaleX_ * (1.0f - lerpVal_));

					damageHP->transform.localPosition.x = currentPosX;
					damageHP->transform.localScale.x = currentScaleX;
				}
				else if (damageDelayTimer_ >= 0.0f)
				{
					damageDelayTimer_ -= g_gameTime->GetFrameDeltaTime();
				}
			}

			/** レベル数値の表示 */
			{
				auto levelDigit = layout_->GetMenu()->GetUI<app::ui::UIDigit>(Hash32("levelNumbers"));
				if (levelDigit)
				{
					//levelDigit->SetZeroPadding(true);
					levelDigit->SetNumber(level_);
				}
			}
			layout_->Update();
		}

		void PlayerHpUIObject::Render(RenderContext& rc)
		{
			if (layout_) {
				layout_->Render(rc);
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

			if (parameter)
			{
				damagePosX_ = parameter->enemyHpBarPositionX[0];
				damageScaleX_ = parameter->enemyHpBarScaleX[0];
			}

			layout_ = std::make_unique<app::ui::Layout>();
			layout_->Initialize <app::ui::MenuBase>("Assets/ui/layout/enemyHpLayout.json");
		}

		EnemyHpUIObject::~EnemyHpUIObject()
		{
		}

		void EnemyHpUIObject::Update()
		{
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
				//moveToScreen(enemyDmgHP);
				//moveToScreen(enemyCurHP);

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
						damagePosX_ = enemyDmgHP->transform.localPosition.x - screenPos.x;
						damageScaleX_ = enemyDmgHP->transform.localScale.x;
					}
					hpIndex_ = newIndex;
				}
				// 即時反映
				if (g_pad[0]->IsTrigger(enButtonUp))
				{
					hpIndex_ = max(0, hpIndex_ - 1);
					// タイマーリセット
					damageDelayTimer_ = 0.3;
					lerpVal_ = 0.0f;

					// 現在地を保存
					damagePosX_ = enemyDmgHP->transform.localPosition.x - screenPos.x;
					damageScaleX_ = enemyDmgHP->transform.localScale.x;
				}
				// 即時反映
				if (g_pad[0]->IsTrigger(enButtonDown))
				{
					hpIndex_ = min(MAX_LEVEL, hpIndex_ + 1);
					// タイマーリセット
					damageDelayTimer_ = 0.3;
					lerpVal_ = 0.0f;
					// 現在地を保存
					damagePosX_ = enemyDmgHP->transform.localPosition.x - screenPos.x;
					damageScaleX_ = enemyDmgHP->transform.localScale.x;
				}
				// 【重要】スクリーン座標に、パラメータのオフセット値（PosX）を足し算する
				float targetOffsetX = parameter->enemyHpBarPositionX[hpIndex_];
				float targetScaleX = parameter->enemyHpBarScaleX[hpIndex_];
				enemyCurHP->transform.localPosition.x = screenPos.x + targetOffsetX;
				enemyCurHP->transform.localPosition.y = screenPos.y;
				enemyCurHP->transform.localScale.x = targetScaleX;



				// --- ダメージバー(Lerp)の更新処理 ---
				// ディレイタイマーとLerp値の更新
				if (lerpVal_ < 1.0f && damageDelayTimer_ < 0.0f)
				{
					// Lerpのスピード
					lerpVal_ += 1.0f * g_gameTime->GetFrameDeltaTime();
					lerpVal_ = min(lerpVal_, 1.0f);

					// 開始地点（保存した値）と目標地点（現在のHPパラメータ）を補間
					float currentDmgOffsetX = (targetOffsetX * lerpVal_) + (damagePosX_ * (1.0f - lerpVal_));
					float currentDmgScaleX = (targetScaleX * lerpVal_) + (damageScaleX_ * (1.0f - lerpVal_));

					// ダメージバーに適用（スクリーン座標 ＋ 補間したオフセット）
					enemyDmgHP->transform.localPosition.x = screenPos.x + currentDmgOffsetX;
					enemyDmgHP->transform.localScale.x = currentDmgScaleX;
				}
				else if (damageDelayTimer_ >= 0.0f)
				{
					// ディレイ中はタイマーを減らす
					damageDelayTimer_ -= g_gameTime->GetFrameDeltaTime();

					// タイマー消化中も敵は動くので、位置はスクリーン座標＋保存したオフセットで追従させる
					enemyDmgHP->transform.localPosition.x = screenPos.x + damagePosX_;
					enemyDmgHP->transform.localScale.x = damageScaleX_;
				}
				else
				{
					// Lerp完了後、何もない平時
					enemyDmgHP->transform.localPosition.x = screenPos.x + targetOffsetX;
					enemyDmgHP->transform.localScale.x = targetScaleX;
				}
				// 赤バーのY座標も忘れずに追従させる
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
}
}
