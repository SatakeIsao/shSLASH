#pragma once

#include "geometry/AABB.h"
#include "SceneLight.h"
#include "IRenderer.h"
#include "Shadow.h"
namespace nsK2EngineLow
{
	//class RenderingEngine;
	class ModelRender : public IRenderer
	{
	public:
		//ディレクションライトの構造体
		struct DirectionLight
		{
			Vector3 direction;
			float pad0;
			//int castShadow = true;
			Vector3 color;
			float pad1;
		};

		//ポイントライトの構造体
		struct PointLight
		{
			Vector3 ptPosition;			// 位置
			float pd2;
			Vector3 ptColor;			// カラー
			float ptRange;				// 影響範囲

		};

		//ライトの構造体
		struct Light
		{
			DirectionLight directionalLight[4];	//ディレクションライトの配列
			PointLight pointLight[4];	//ポイントライト
			Vector3 eyePos;		//視点の位置
			float pad1;
			Vector3 ambientLight;	//環境光
			float pad2;
			Matrix m_mt;
		};

		ModelRender();
		~ModelRender();

		
		void Init(
			const char* tkmFilePath,
			AnimationClip* animationClip = nullptr,
			int numAnimationClips = 0,
			EnModelUpAxis enModelUpAxis = enModelUpAxisZ,
			bool m_isShadowCaster = true,
			bool m_isShadowReceiver = true
			);

		void InitShadowModel(
			const char* tkmFilePath,
			EnModelUpAxis modelUpAxis
		);

		//void InitInstancingShadowDrawModel(
		//	const char* tkmFilePath,
		//	EnModelUpAxis enModelUpAxis);
		void Update();
		/// <summary>
		/// インスタンシングデータの更新
		/// </summary>
		/// <param name="instanceNo">インスタンス番号</param>
		/// <param name="pos">座標</param>
		/// <param name="rot">回転</param>
		/// <param name="scale">拡大率</param>
		//void UpdateInstancingData(int instanceNo, const Vector3& pos, const Quaternion& rot, const Vector3& scale);
		//描画処理
		void Draw(RenderContext& rc);
		
		//シャドウマップへの描画パスから呼ばれる処理
		void OnRenderShadowMap(RenderContext& rc, const Matrix& lvpMatrix)override;
		/// <summary>
		/// モデルの描画
		/// </summary>
		/// <param name="rc">RenderContext</param>
		void OnRenderModel(RenderContext& rc)override;
		//void DirLigMove();
		//void PtLigMove();
		//座標を更新
		void SetPosition(Vector3 pos)
		{
			m_position = pos;
		}
		void SetPosition(float x, float y, float z)
		{
			SetPosition({ x,y,z });
		}
		/// <summary>
		/// 回転設定
		/// </summary>
		/// <param name="rot"></param>
		void SetRotation(const Quaternion& rot)
		{
			m_rotation = rot;
		}
		

		/// <summary>
		/// サイズ設定
		/// </summary>
		/// <param name="scale"></param>
		void SetScale(const Vector3& scale)
		{
			m_scale = scale;
		}
		void SetScale(const float x, const float y, const float z)
		{
			SetScale({ x,y,z });
		}
		void SetScale(const float xyz)
		{
			SetScale({ xyz,xyz,xyz });
		}

		/// <summary>
		/// 座標・拡大・回転を設定
		/// </summary>
		/// <param name="pos"></param>
		/// <param name="rot"></param>
		/// <param name="scale"></param>
		void SetTRS(const Vector3& pos, const Quaternion& rot, const Vector3& scale)
		{
			SetPosition(pos);
			SetRotation(rot);
			SetScale(scale);
		}

		const Matrix& GetLigCameraViewProjection()
		{
			return m_shadow.GetLigCameraViewProjection();
		}

		//モデルを取得
		Model& GetModel()
		{
			return m_model;
		}

		//モデルの座標を取得
		Vector3& GetPosition()
		{
			return m_position;
		}

		//モデルのX座標を取得
		float& GetPositionX()
		{
			return m_position.x;
		}

		//モデルのY座標を取得
		float& GetPositionY()
		{
			return m_position.y;
		}

		//モデルのZ座標を取得
		float& GetPositionZ()
		{
			return m_position.z;
		}

		/// <summary>
		/// スケルトンの初期化
		/// </summary>
		/// <param name="filePath"></param>
		void InitSkeleton(const char* filePath);
		/// <summary>
		/// アニメーションの初期化
		/// </summary>
		/// <param name="animationClips"></param>
		/// <param name="numAnimationClips"></param>
		/// <param name="enModelUpAxis"></param>
		void InitAnimation(
			AnimationClip* animationClips,
			int numAnimationClips,
			EnModelUpAxis enModelUpAxis
		);
		/*/// <summary>
		/// アニメーション済み頂点バッファの計算処理を初期化
		/// </summary>
		/// <param name="tkmFilePath"></param>
		/// <param name="enModelUpAxis"></param>
		void InitComputeAnimationVertexBuffer(
			const char* tkmFilePath,
			EnModelUpAxis enModelUpAxis
		);*/

		/// <summary>
		/// アニメーション再生
		/// </summary>
		/// <param name="animNo">アニメーションクリップの番号</param>
		/// <param name="interpolateTime">補間時間（単位:秒）</param>
		void PlayAnimation(int animNo, float interpolateTime = 0.0f)
		{
			m_animation.Play(animNo, interpolateTime);
		}
		//アニメーション再生中？
		bool IsPlayingAnimation() const
		{
			return m_animation.IsPlaying();
		}
		//アニメーション再生の速度を設定
		void SetAnimationSpeed(const float animationSpeed)
		{
			m_animationSpeed = animationSpeed;
		}
		//アニメーションイベント
		void AddAnimationEvent(AnimationEventListener eventListener)
		{
			m_animation.AddAnimationEventListener(eventListener);
		}

		/// <summary>
		/// スカイキューブの初期化
		/// </summary>
		/// <param name="initData"></param>
		void InitSkyCubeModel(ModelInitData& initData)
		{
			m_model.Init(initData);
		}

		/// <summary>
		/// シャドウキャスターのフラグを設定する
		/// </summary>
		/// <param name="flag"></param>
		void SetShadowCasterFlag(bool flag)
		{
			m_isShadowCaster = flag;
		}

		/// <summary>
		/// シャドウキャスター？
		/// </summary>
		/// <returns></returns>
		bool IsShadowCaster() const
		{
			return m_isShadowCaster;
		}

		/// <summary>
		/// カリングの対象から外す
		/// </summary>
		void SetDontCulling()
		{
			m_doCulling = false;
		}
	private:
		Model m_model;					//モデル
		//Model m_bgModel;
		Model m_renderToGBufferModel;	//RenderToGBufferで描画されるモデル
		Model m_shadowModel;			//シャドウマップ
		Light m_light;					//シェーダーに送るライトの情報
		Vector3 m_position = Vector3::Zero;		//座標
		Quaternion m_rotation;			//回転
		Vector3 m_scale = g_vec3One;	//拡大率
		Shadow m_shadow;
		Skeleton m_skeleton;
		AnimationClip* m_animationClips = nullptr;
		int m_numAnimationClips = 0;
		Animation m_animation;
		//ComputeAnimationVertexBuffer m_computeAnimationVertexBuffer;	// アニメーション済み頂点バッファの計算処理。
		float m_animationSpeed = 1.0f; //アニメーションの再生スピード
		//RenderTarget m_shadowMap;
		int m_moveState = 0;
		bool m_isEnableInstancingDraw = false; //インスタンシング描画が有効か？
		bool m_isRaytrecingWorld = true;	   //レイトレワールドに登録するか？
		StructuredBuffer m_worldMatrixArraySB; //ワールド行列の配列のストラクチャードバッファ
		bool m_isShadowCaster = false;		   //シャドウキャスターフラグ
		bool m_doCulling = true;			   //カリングするかどうか

		int							m_maxInstance = 1;					// 最大インスタンス数。
		Model						m_translucentModel;					// 半透明モデル。
		Model						m_zprepassModel;					// ZPrepassで描画されるモデル
		int							m_numInstance = 0;					// インスタンスの数。



		std::unique_ptr<Matrix[]>	m_worldMatrixArray;					// ワールド行列の配列。
		std::unique_ptr<int[]>		m_instanceNoToWorldMatrixArrayIndexTable;	// インスタンス番号からワールド行列の配列のインデックスに変換するテーブル。
	};
}

