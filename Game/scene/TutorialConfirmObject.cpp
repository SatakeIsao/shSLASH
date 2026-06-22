#include "stdafx.h"
#include "TutorialConfirmObject.h"

void TutorialConfirmObject::Render(RenderContext& rc)
{
    if (isActive_ && layout_)
    {
        layout_->Render(rc);
    }
}

void TutorialConfirmObject::Activate(app::ui::Layout* layout)
{
    layout_ = layout;
    isActive_ = true;
}

void TutorialConfirmObject::Deactivate()
{
    isActive_ = false;
}
