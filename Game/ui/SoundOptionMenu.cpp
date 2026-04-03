#include "stdafx.h"
#include "SoundOptionMenu.h"
#include "battle/BattleManager.h"
#include "sound/SoundManager.h"
#include "core/ParameterManager.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"


namespace 
{
	/** 音量増減の1目盛 */
	constexpr float VOLUME_STEP = 0.10f;
	/** 最大音量(0.0～1.0の範囲) */
	constexpr float VOLUME_MAX = 1.0f;
	/** 最小音量(0.0～1.0の範囲) */
	constexpr float VOLUME_MIN = 0.0f;
	/** デフォルト音量 */
	constexpr float VOLUME_DEFAULT = 0.50f;
	/** 数値表示用に0.0～1.0の音量を0～10に変換するための乗数 */
	constexpr float VOLUME_DISPLAY_MULTIPLIER = 10.0f;

	// @todo for test
	static app::ui::UIAnimationSequence* seq = nullptr;
}


namespace app
{
	namespace ui
	{
		SoundOptionMenu::SoundOptionMenu()
			: volumeCursolIndex_(static_cast<int>(app::SoundManager::SoundVolumeType::Master))
		{
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterSoundOptionMenuParameter>("Assets/master/SoundOptionMenuParameter.json", [](const nlohmann::json& j, app::core::MasterSoundOptionMenuParameter& p)
				{
					// 多いからいい感じにした
					char gaugeBarXStr[] = "gaugeBarXA";
					const uint32_t barYSize = ARRAYSIZE(p.gaugeBarX);
					for (uint32_t i = 0; i < barYSize; ++i) {
						gaugeBarXStr[9] = 'A' + i;
						p.gaugeBarX[i] = j[gaugeBarXStr];
					}
					
					// 2個しかないのでそのまま
					p.gaugeBarY[0] = j["gaugeBarYA"];
					p.gaugeBarY[1] = j["gaugeBarYB"];
					p.gaugeBarY[2] = j["gaugeBarYC"];

					//多い
					char gaugeScaleSE[] = "gaugeBarScaleXA";
					const uint32_t barScaleX = ARRAYSIZE(p.gaugeBarScaleX);
					for (uint32_t i = 0; i < barScaleX; ++i) {
						gaugeScaleSE[14] = 'A' + i;
						p.gaugeBarScaleX[i] = j[gaugeScaleSE];
					}

					//ノブ
					char knobSE[] = "knobXA";
					const uint32_t knobX = ARRAYSIZE(p.knobX);
					for (uint32_t i = 0; i < knobX; ++i) {
						knobSE[5] = 'A' + i;
						p.knobX[i] = j[knobSE];
					}
				});
		}


		SoundOptionMenu::~SoundOptionMenu()
		{
			app::core::ParameterManager::Get().UnloadParameter<app::core::MasterSoundOptionMenuParameter>();
		}


		void SoundOptionMenu::Update()
		{
			auto* canvas = GetCanvas();
			if (canvas)
			{
				//閉じる
				{
					auto* closeAnim = canvas->FindAnimation(Hash32("ScaleDown"));
					if (closeAnim && !closeAnim->IsPlay())
					{
						canvas->RemoveAnimation(Hash32("ScaleDown"));
						closeAnim = nullptr;
						isPause_ = false;

						//ForEachUI([](app::ui::UIBase* ui)
						//	{
						//		ui->RemoveAnimation(Hash32("FadeOutPauseMenu"));
						//	});
					}
				}
				//開く
				{
					auto* openAnim = canvas->FindAnimation(Hash32("ScaleUp"));
					if (openAnim && !openAnim->IsPlay())
					{
						canvas->RemoveAnimation(Hash32("ScaleUp"));
						
						//ForEachUI([](app::ui::UIBase* ui) {
						//	ui->RemoveAnimation(Hash32("FadeInPauseMenu"));
						//	});
					}
				}
			}

			if (isPause_)
			{
				if (g_pad[0]->IsTrigger(enButtonUp))
				{
					app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));

					volumeCursolIndex_--;
					if (volumeCursolIndex_ < static_cast<int>(app::SoundManager::SoundVolumeType::Master))
					{
						volumeCursolIndex_ = static_cast<int>(app::SoundManager::SoundVolumeType::Master);
					}
				}
				else if (g_pad[0]->IsTrigger(enButtonDown))
				{
					app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));

					volumeCursolIndex_++;
					if (volumeCursolIndex_ >= static_cast<int>(app::SoundManager::SoundVolumeType::Max))
					{
						volumeCursolIndex_ = static_cast<int>(app::SoundManager::SoundVolumeType::SE);
					}
				}

				/** 左右キーで音量調整 */
				bool isVolumeChanged = false;
				
				auto currentVolumeType = static_cast<app::SoundManager::SoundVolumeType>(volumeCursolIndex_);
				float targetVolume = app::SoundManager::Get().GetVolume(currentVolumeType);

				if (g_pad[0]->IsTrigger(enButtonRB1))
				{
					app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
					targetVolume += VOLUME_STEP;
					if (targetVolume > VOLUME_MAX)
					{
						targetVolume = VOLUME_MAX;
					}
					isVolumeChanged = true;
				}
				else if (g_pad[0]->IsTrigger(enButtonLB1))
				{
					app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
					targetVolume -= VOLUME_STEP;
					if (targetVolume < VOLUME_MIN)
					{
						targetVolume = VOLUME_MIN;
					}
					isVolumeChanged = true;
				}

				if (isVolumeChanged)
				{
					app::SoundManager::Get().SetVolume(currentVolumeType, targetVolume);
				}

				// 動的に数値をUIに設定
				auto* parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterSoundOptionMenuParameter>();

				// MASTER/BGM/SEの場所
				{
					const float y = parameter->gaugeBarY[volumeCursolIndex_];
					auto cursol = GetUI<UIIcon>(Hash32("Cursol"));
					cursol->transform.localPosition.y = y;
				}
				// --- MASTERのUI更新
				{
					const float volumeMaster = app::SoundManager::Get().GetVolume(app::SoundManager::SoundVolumeType::Master);
					// ゲージの大きさ
					{
						auto gaugeMASTER = GetUI<UIIcon>(Hash32("VolumeBar_MASTER"));
						gaugeMASTER->transform.localScale.x = volumeMaster;
					}
					// MASTERノブのX座標
					{
						const float minX = parameter->gaugeBarX[0];
						const float maxX = parameter->gaugeBarX[10];

						auto knobMASTER = GetUI<UIIcon>(Hash32("Knob_MASTER"));
						const float x = minX + ((minX - maxX) * volumeMaster);
						knobMASTER->transform.localPosition.x = x;
					}
					//数値表示
					{
						auto digitMASTER = GetUI<UIDigit>(Hash32("VolumeDigit_MASTER"));
						if (digitMASTER) {
							digitMASTER->SetNumber(static_cast<int>(std::round(volumeMaster * VOLUME_DISPLAY_MULTIPLIER)));
						}
					}
				}

				// --- BGMのUI更新
				{
					const float volumeBGM = app::SoundManager::Get().GetVolume(app::SoundManager::SoundVolumeType::BGM);
					// ゲージの大きさ
					{
						auto gaugeBGM = GetUI<UIIcon>(Hash32("VolumeBar_BGM"));
						gaugeBGM->transform.localScale.x = volumeBGM;
					}
					// BGMノブのX座標
					{
						const float minX = parameter->gaugeBarX[0];
						const float maxX = parameter->gaugeBarX[10];

						auto knobBGM = GetUI<UIIcon>(Hash32("Knob_BGM"));
						const float x = minX + ((minX - maxX) * volumeBGM);
						knobBGM->transform.localPosition.x = x;
					}
					//数値表示
					{
						auto digitBGM = GetUI<UIDigit>(Hash32("VolumeDigit_BGM"));
						if (digitBGM) {
							digitBGM->SetNumber(static_cast<int>(std::round(volumeBGM * VOLUME_DISPLAY_MULTIPLIER)));
						}
					}
				}

				// --- SEのUI更新
				{
					const float volumeSE = app::SoundManager::Get().GetVolume(app::SoundManager::SoundVolumeType::SE);
					// ゲージの大きさ
					{
						auto gaugeScaleSE = GetUI<UIIcon>(Hash32("VolumeBar_SE"));
						gaugeScaleSE->transform.localScale.x = volumeSE;
					}
					// SEノブのX座標
					{
						const float minX = parameter->gaugeBarX[0];
						const float maxX = parameter->gaugeBarX[10];

						// 四角の場所 ＝ minX + ((maxX - minX) * ボリュームパーセント)
						// ※ (maxX - minX) = 長さ

						auto knobSE = GetUI<UIIcon>(Hash32("Knob_SE"));
						const float x = minX + ((minX - maxX) * volumeSE);
						knobSE->transform.localPosition.x = x;
					}
					//数値の表示
					{
						auto digitSE = GetUI<UIDigit>(Hash32("VolumeDigit_SE"));
						if (digitSE) {
							digitSE->SetNumber(static_cast<int>(std::round(volumeSE * VOLUME_DISPLAY_MULTIPLIER)));
						}
					}
				}
			}

			seq->Update(g_gameTime->GetFrameDeltaTime());

			MenuBase::Update();
		}


		void SoundOptionMenu::OnOpen()
		{
			app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));
			isPause_ = true;

			//キャンバス
			{
				auto* canvas = GetCanvas();
				if (canvas)
				{
					canvas->RemoveAnimation(Hash32("ScaleUp"));
					canvas->RemoveAnimation(Hash32("ScaleDown"));
					//アニメーションをアタッチ
					app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(canvas, Hash32("ScaleUp"));
					auto* openAnim = canvas->FindAnimation(Hash32("ScaleUp"));
					if (openAnim) openAnim->Play();
				}
			}

			//各UIパーツの一斉フェードイン
			{
				ForEachUI([](app::ui::UIBase* ui)
					{
						ui->RemoveAnimation(Hash32("FadeInPauseMenu"));
						ui->RemoveAnimation(Hash32("FadeOutPauseMenu"));
						app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(ui, Hash32("FadeInPauseMenu"));
						auto* openAnim = ui->FindAnimation(Hash32("FadeInPauseMenu"));
						//if (openAnim) openAnim->Play();
					});
			}
		}


		void SoundOptionMenu::OnClose()
		{
			app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Button));

			// キャンバス
			{
				auto* canvas = GetCanvas();
				if (canvas)
				{
					canvas->RemoveAnimation(Hash32("ScaleUp"));
					canvas->RemoveAnimation(Hash32("ScaleDown"));
					//アニメーションをアタッチ
					app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(canvas, Hash32("ScaleDown"));
					auto* closeAnim = canvas->FindAnimation(Hash32("ScaleDown"));
					if (closeAnim) closeAnim->Play();
				}
			}

			//各UIパーツの一斉フェードアウト
			{
				ForEachUI([](app::ui::UIBase* ui) {
					ui->RemoveAnimation(Hash32("FadeInPauseMenu"));
					ui->RemoveAnimation(Hash32("FadeOutPauseMenu"));
					app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(ui, Hash32("FadeOutPauseMenu"));
					auto* closeAnim = ui->FindAnimation(Hash32("FadeOutPauseMenu"));
					//if (closeAnim) closeAnim->Play();
					});
			}
			
		}


		void SoundOptionMenu::InitializeLogic()
		{
			// サウンドバーの位置情報設定
			// アニメーションとかいれたり
			/** キャンバス（UI全体) */
			{
				auto* canvas = GetCanvas();
				if (canvas)
				{
					//canvas->color.w = 0.0f;
					canvas->transform.localScale = Vector3::Zero;
				}
			}

			/** カーソルUI */
			{
				auto* cursol = GetUI<app::ui::UIIcon>(Hash32("Cursol"));
				if (cursol)
				{
					// アニメーションをアタッチ
					app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(cursol, Hash32("FadeIn"));
					app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(cursol, Hash32("FadeOut"));
					app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(cursol, Hash32("ScaleUp"));
					
					
					seq = new app::ui::UIAnimationSequence();
					seq->Add(Hash32("FadeIn"));
						//.Add(Hash32("ScaleUp"))
						//.Add(Hash32("FadeOut"));
					

					// アニメーションを再生
					seq->Play(cursol);
				}
			}

			// バー
			{
				// MASTER
				{
					auto* gauge = GetUI<app::ui::UIIcon>(Hash32("VolumeBar_MASTER"));
					gauge->SetPivot(Vector2(0.0f, 0.5f));
				}
				//  //DEBUGTEST_Pivot
				//  {
				//  	auto* se = GetUI<app::ui::UIDigit>(Hash32("VolumeDigit_MASTER"));
				//  	//se->Set
				//  }
				// BGM
				{
					auto* gauge = GetUI<app::ui::UIIcon>(Hash32("VolumeBar_BGM"));
					gauge->SetPivot(Vector2(0.0f, 0.5f));
				}
				// SE
				{
					auto* gauge = GetUI<app::ui::UIIcon>(Hash32("VolumeBar_SE"));
					gauge->SetPivot(Vector2(0.0f, 0.5f));
				}
			}
			//初期位置
			{
				app::SoundManager::Get().SetVolume(app::SoundManager::SoundVolumeType::Master, VOLUME_DEFAULT);
				app::SoundManager::Get().SetVolume(app::SoundManager::SoundVolumeType::BGM, VOLUME_DEFAULT);
				app::SoundManager::Get().SetVolume(app::SoundManager::SoundVolumeType::SE, VOLUME_DEFAULT);
			}
		}
	}
}