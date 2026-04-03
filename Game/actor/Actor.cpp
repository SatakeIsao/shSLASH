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
			modelRender_->SetTRS(transform.position, transform.rotation, transform.scale);
			modelRender_->Update();
		}


		void Character::Render(RenderContext& rc)
		{
			modelRender_->Draw(rc);
		}
	}
}