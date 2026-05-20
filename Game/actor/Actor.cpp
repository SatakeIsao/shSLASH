/**
 * Actorファイル
 */
#include "stdafx.h"
#include "Actor.h"


namespace app
{
	namespace actor
	{
		Character::Character()
		{
		}


		Character::~Character()
		{
			delete status_;
			status_ = nullptr;

			modelRender_.reset();
		}

		void Character::Update()
		{
			if (modelRender_ == nullptr) { return; }
			modelRender_->SetTRS(transform.position, transform.rotation, transform.scale);
			modelRender_->Update();
		}


		void Character::Render(RenderContext& rc)
		{
			if (modelRender_ == nullptr) { return; }
			modelRender_->Draw(rc);
		}
	}
}