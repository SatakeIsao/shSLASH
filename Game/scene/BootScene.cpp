/**
 * BootScene.cpp
 * 起動画面のシーン
 */
#include "stdafx.h"
#include "BootScene.h"

#if defined(APP_DEBUG)

#include "StartupScene.h"
#include "DebugScene.h"
#include "GameOverScene.h"


namespace
{
	struct SelectSceneInformation
	{
		const wchar_t* name;
		Vector3 position;
		uint32_t sceneId;
		//
		SelectSceneInformation(const wchar_t* name, const Vector3& pos, const uint32_t id)
			: name(name)
			, position(pos)
			, sceneId(id)
		{
		}
	};


	static const SelectSceneInformation selectSceneinformationList[static_cast<uint8_t>(SceneKind::Max)] =
	{
		SelectSceneInformation(L"デバッグ", Vector3(0.0f, 100.0f, 0.0f), DebugScene::ID()),
		SelectSceneInformation(L"スタートアップ", Vector3(0.0f, 200.0f, 0.0f), StartupScene::ID()),
		SelectSceneInformation(L"ゲームオーバー",Vector3(0.0f,300.0f,0.0f),GameOverScene::ID()),
	};


	static const Vector3 SELECTOR_POSITON_LIST[static_cast<uint8_t>(SceneKind::Max)] =
	{
		Vector3(-50.0f, 190.0f, 0.0f),
		Vector3(-50.0f, 80.0f, 0.0f),
		Vector3(-50.0f, -30.0f,0.0f)
	};
}


BootScene::BootScene()
{
}


BootScene::~BootScene()
{
}


bool BootScene::Start()
{
	// シーン選択用に表示。どんなシーンがあるか
	for (uint8_t i = 0; i < static_cast<uint8_t>(SceneKind::Max); i++) {
		const auto& info = selectSceneinformationList[i];
		m_sceneText[i].SetText(info.name);
		m_sceneText[i].SetPosition(info.position);
	}
	// 選択している個所を分かるようにするアイコン
	//m_selecterRender.Init("Assets/modelData/debug/selecter.dds", 64.0f, 64.0f);

	return true;
}


void BootScene::Update()
{
	if (g_pad[0]->IsTrigger(enButtonUp)) {
		m_selectIndex--;
		// 最小値チェック
		if (m_selectIndex < static_cast<uint8_t>(SceneKind::Default)) {
			m_selectIndex = static_cast<uint8_t>(SceneKind::Default);	// 範囲外にならないように調整
		}
	}
	else if (g_pad[0]->IsTrigger(enButtonDown)) {
		m_selectIndex++;
		// 最大値チェック
		if (m_selectIndex >= static_cast<uint8_t>(SceneKind::Max)) {
			m_selectIndex = static_cast<uint8_t>(SceneKind::Max) - 1;	// 範囲外にならないように調整
		}
	}

	// シーン設定
	if (g_pad[0]->IsTrigger(enButtonA)) {
		const auto& info = selectSceneinformationList[m_selectIndex];
		m_requestSceneId = info.sceneId;
	}

	//m_selecterRender.SetPosition(SELECTOR_POSITON_LIST[m_selectIndex]);
	//m_selecterRender.Update();
}


void BootScene::Render(RenderContext& rc)
{
	// シーンの名前描画
	for (uint8_t i = 0; i < static_cast<uint8_t>(SceneKind::Max); i++)
	{
		m_sceneText[i].Draw(rc);
	}
	// セレクター描画
	//m_selecterRender.Draw(rc);
}


bool BootScene::RequestScene(uint32_t& id, float& waitTime)
{
	if (m_requestSceneId != INVALID_SCENE_ID) {
		id = m_requestSceneId;
		waitTime = 0.0f;
		return true;
	}
	return false;
}

#endif // APP_DEBUG