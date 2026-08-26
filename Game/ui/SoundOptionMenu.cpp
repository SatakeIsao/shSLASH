#include "stdafx.h"
#include "SoundOptionMenu.h"
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
	constexpr float VOLUME_DEFAULT_MASTER = 0.50f;
	constexpr float VOLUME_DEFAULT_BGM = 0.50f;
	constexpr float VOLUME_DEFAULT_SE = 0.50f;
	/** 数値表示用に0.0～1.0の音量を0～10に変換するための乗数 */
	constexpr float VOLUME_DISPLAY_MULTIPLIER = 10.0f;
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

		void SoundOptionMenu::InitializeLogic()
		{
			// サウンドバーの位置情報設定
			// アニメーションとかいれたり
			/** キャンバス（UI全体) */
			auto* canvas = GetCanvas();
			if (canvas)
			{
				canvas->transform.localScale = Vector3::One;
				canvas->transform.UpdateTransform();
			}

			ForEachUI([](app::ui::UIBase* ui) {
				ui->color.w = 1.0f; // 透明度100%
				ui->isDraw = true;  // 描画オン
			});

			auto* cursol = GetUI<app::ui::UIIcon>(Hash32("Cursol"));
			if (cursol)
			{
				app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(cursol, Hash32("FadeIn"));
			}


			auto* gaugeMaster = GetUI<app::ui::UIIcon>(Hash32("VolumeBar_MASTER"));
			if (gaugeMaster) gaugeMaster->SetPivot(Vector2(0.0f, 0.5f));

			auto* gaugeBgm = GetUI<app::ui::UIIcon>(Hash32("VolumeBar_BGM"));
			if (gaugeBgm) gaugeBgm->SetPivot(Vector2(0.0f, 0.5f));

			auto* gaugeSe = GetUI<app::ui::UIIcon>(Hash32("VolumeBar_SE"));
			if (gaugeSe) gaugeSe->SetPivot(Vector2(0.0f, 0.5f));

			app::SoundManager::Get().SetVolume(app::SoundManager::SoundVolumeType::Master, VOLUME_DEFAULT_MASTER);
			app::SoundManager::Get().SetVolume(app::SoundManager::SoundVolumeType::BGM, VOLUME_DEFAULT_BGM);
			app::SoundManager::Get().SetVolume(app::SoundManager::SoundVolumeType::SE, VOLUME_DEFAULT_SE);

		}

		void SoundOptionMenu::SetDraw(bool isDraw)
		{
			auto* canvas = GetCanvas();
			if (canvas) {
				canvas->isDraw = isDraw;
			}
			auto* cursol = GetUI<app::ui::UIIcon>(Hash32("Cursol"));
			if (isDraw) {
				if (cursol) {
					auto* anim = cursol->FindAnimation(Hash32("FadeIn"));
					if (anim) anim->Play();
				}
			}
			else {
				if (cursol) cursol->StopSpriteAnimation();
			}
		}

		void SoundOptionMenu::Update()
		{
			auto* canvas = GetCanvas();
			if (!canvas || !canvas->isDraw) return;

			MenuBase::Update();

			if (g_pad[0]->IsTrigger(enButtonUp))
			{
				app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonMove));

				volumeCursolIndex_--;

				if (volumeCursolIndex_ < static_cast<int>(app::SoundManager::SoundVolumeType::Master))
				{
					volumeCursolIndex_ = static_cast<int>(app::SoundManager::SoundVolumeType::SE);
				}
			}
			else if (g_pad[0]->IsTrigger(enButtonDown))
			{
				app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonMove));

				volumeCursolIndex_++;

				if (volumeCursolIndex_ >= static_cast<int>(app::SoundManager::SoundVolumeType::Max))
				{
					volumeCursolIndex_ = static_cast<int>(app::SoundManager::SoundVolumeType::Master);
				}
			}


			/** 左右キーで音量調整 */
			bool isVolumeChanged = false;
			auto currentVolumeType = static_cast<app::SoundManager::SoundVolumeType>(volumeCursolIndex_);
			float targetVolume = app::SoundManager::Get().GetVolume(currentVolumeType);

			if (g_pad[0]->IsTrigger(enButtonY))
			{
				app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonReset));
				targetVolume = 0.5f;
				isVolumeChanged = true;
			}
			if (g_pad[0]->IsTrigger(enButtonRight))
			{
				if (targetVolume < VOLUME_MAX)
				{
					app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonMove));
					targetVolume += VOLUME_STEP;
					if (targetVolume > VOLUME_MAX)
					{
						targetVolume = VOLUME_MAX;
					}
					isVolumeChanged = true;
				}
			}
			else if (g_pad[0]->IsTrigger(enButtonLeft))
			{
				if (targetVolume > VOLUME_MIN)
				{
					app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::ButtonMove));
					targetVolume -= VOLUME_STEP;
					if (targetVolume < VOLUME_MIN)
					{
						targetVolume = VOLUME_MIN;
					}
					isVolumeChanged = true;
				}
			}

			if (isVolumeChanged)
			{
				app::SoundManager::Get().SetVolume(currentVolumeType, targetVolume);

				//if (app::ui::AwardManager::IsAvailable()) {
				//	app::ui::AwardManager::Get().OnSoundAdjusted();
				//}
			}

			// 動的に数値をUIに設定
			auto* parameter = app::core::ParameterManager::Get().GetParameter<app::core::MasterSoundOptionMenuParameter>();
			if (!parameter) return;

			// MASTER/BGM/SEの場所
			{
				const float y = parameter->gaugeBarY[volumeCursolIndex_];
				auto cursol = GetUI<UIIcon>(Hash32("Cursol"));
				if (cursol) cursol->transform.localPosition.y = y;
			}
			// --- MASTERのUI更新
			{
				const float volumeMaster = app::SoundManager::Get().GetVolume(app::SoundManager::SoundVolumeType::Master);

				int volIndex = static_cast<int>(std::round(volumeMaster * VOLUME_DISPLAY_MULTIPLIER));
				if (volIndex < 0) volIndex = 0;
				if (volIndex > 10) volIndex = 10;

				// ゲージの大きさ
				auto gaugeMASTER = GetUI<UIIcon>(Hash32("VolumeBar_MASTER"));
				if (gaugeMASTER) {
					gaugeMASTER->transform.localScale.x = parameter->gaugeBarScaleX[volIndex];
				}
				auto knobBackGroundMASTER = GetUI<UIIcon>(Hash32("KnobBackground_MASTER"));
				if (knobBackGroundMASTER) {
					knobBackGroundMASTER->transform.localPosition.x = parameter->knobX[volIndex];
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
				int volIndex = static_cast<int>(std::round(volumeBGM * VOLUME_DISPLAY_MULTIPLIER));
				if (volIndex < 0) volIndex = 0;
				if (volIndex > 10) volIndex = 10;

				auto gaugeBGM = GetUI<UIIcon>(Hash32("VolumeBar_BGM"));
				if (gaugeBGM) {
					gaugeBGM->transform.localScale.x = parameter->gaugeBarScaleX[volIndex];
				}

				auto knobBackGroundBGM = GetUI<UIIcon>(Hash32("KnobBackground_BGM"));
				if (knobBackGroundBGM) {
					knobBackGroundBGM->transform.localPosition.x = parameter->knobX[volIndex];
				}

				auto digitBGM = GetUI<UIDigit>(Hash32("VolumeDigit_BGM"));
				if (digitBGM) {
					digitBGM->SetNumber(volIndex);
				}
			}

			// --- SEのUI更新
			{
				const float volumeSE = app::SoundManager::Get().GetVolume(app::SoundManager::SoundVolumeType::SE);
				int volIndex = static_cast<int>(std::round(volumeSE * VOLUME_DISPLAY_MULTIPLIER));
				if (volIndex < 0) volIndex = 0;
				if (volIndex > 10) volIndex = 10;

				auto gaugeScaleSE = GetUI<UIIcon>(Hash32("VolumeBar_SE"));
				if (gaugeScaleSE) {
					gaugeScaleSE->transform.localScale.x = parameter->gaugeBarScaleX[volIndex];
				}

				auto knobBackGroundSE = GetUI<UIIcon>(Hash32("KnobBackground_SE"));
				if (knobBackGroundSE) {
					knobBackGroundSE->transform.localPosition.x = parameter->knobX[volIndex];
				}

				auto digitSE = GetUI<UIDigit>(Hash32("VolumeDigit_SE"));
				if (digitSE) {
					digitSE->SetNumber(volIndex);
				}
			}
		}
	}
}