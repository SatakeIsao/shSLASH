#include "stdafx.h"
#include "ui/TutorialConfirmObject.h"

void app::ui::TutorialConfirmObject::Render(RenderContext& rc)
{
    if (isActive_ && layout_)
    {
        layout_->Render(rc);
    }
}

void app::ui::TutorialConfirmObject::Activate(app::ui::Layout* layout)
{
    layout_ = layout;
    isActive_ = true;
}

void app::ui::TutorialConfirmObject::Deactivate()
{
    isActive_ = false;
}
