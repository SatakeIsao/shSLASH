/**
 * Actorファイル
 */
#include "stdafx.h"
#include "Gimmick.h"


namespace app
{
	namespace actor
	{
		IGimmick::IGimmick()
		{}


		IGimmick::~IGimmick()
		{}

		void IGimmick::Update()
		{
			modelRender_->SetTRS(transform.position, transform.rotation, transform.scale);
			modelRender_->Update();
		}


		void IGimmick::Render(RenderContext& rc)
		{
			modelRender_->Draw(rc);
		}




		/************************************/


		StaticGimmick::StaticGimmick()
		{}


		StaticGimmick::~StaticGimmick()
		{}


		bool StaticGimmick::Start()
		{
			return true;
		}


		void StaticGimmick::Update()
		{
			IGimmick::Update();
		}


		void StaticGimmick::Render(RenderContext& rc)
		{
			IGimmick::Render(rc);
		}


		void StaticGimmick::Initialize(const char* path)
		{
			// モデル読み込み
			modelRender_ = std::make_unique<ModelRender>();
			modelRender_->Init(path);
			modelRender_->SetTRS(transform.position, transform.rotation, transform.scale);
			modelRender_->Update();
			// 物理オブジェクト作成
			physicalBody_.reset(new app::collision::PhysicalBody());
			//physicalBody_->CreateFromModel(modelRender_->GetModel(), Matrix::Identity, app::collision::CollisionAttribute::Ground);

			app::collision::Bounds bounds;
			bounds.Compute(modelRender_->GetModel());
			physicalBody_->CreateBox(bounds.maxPoint - bounds.minPoint, transform.position, app::collision::CollisionAttribute::Ground);
		}




		/************************************/
	}
}
