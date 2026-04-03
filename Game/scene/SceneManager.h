#pragma once
#include "Scene/IScene.h"
#include <map>
#include <functional>


/**
 * シーン管理を処理するためのゲームオブジェクト
 */
class SceneManagerObject : public IGameObject
{
public:
	SceneManagerObject();
	~SceneManagerObject();

	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc);
};


/** 
 * シーン管理クラス
 * シーン遷移だったりは、ここを介して行います
 * シングルトンの勉強用で実装してみました
 */
class SceneManager
{
	friend class SceneManagerObject;


private:
	/** 次のシーンID */
	uint32_t nextSceneId_ = INVALID_SCENE_ID;
	
	/** 経過時間 */
	float m_elapsedTime = 0.0f;
	float m_waitTime = 0.0f;


private:
	SceneManager();
	/** 仮想デストラクタ:派生クラスのデストラクタが呼ばれるようにする */
	~SceneManager(); 


public:
	/**
	 * 更新
	 */
	void Update();
	/**
	 * 描画
	 */
	void Render(RenderContext& rc);


private:
	/**
	 * Scene追加を簡易的にする関数
	 */
	template <typename T>
	void AddSceneMap()
	{
		m_sceneMap.emplace(T::ID(), []()
			{
				return new T();
			});
	}
	/**
	 * シーン生成ラッパー関数
	 */
	void CreateScene(const uint32_t id);


private:
	using SceneMap = std::map<uint32_t, std::function<IScene*()>>;
	/** シーンのマップ */
	SceneMap m_sceneMap;		
	/** 現在のシーン */
	IScene* m_currentScene;		
	



	/**
	 * シングルトン用
	 */
public:
	/**
	 * シングルトンインスタンスを生成する
	 */
	static void Initialize()
	{
		if (m_instance == nullptr)
		{
			m_instance = new SceneManager();
		}
	}

	/**
	 * シングルトンインスタンスを取得する
	 */
	static SceneManager& Get()
	{
		return *m_instance;
	}

	/**
	 * シングルトンインスタンスを破棄する 
	 */
	static void Finalize()
	{
		if (m_instance != nullptr)
		{
			delete m_instance;
			m_instance = nullptr;
		}
	}


private:
	/** シングルトンインスタンス */
	static SceneManager* m_instance; 
};