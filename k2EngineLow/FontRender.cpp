#include "k2EngineLowPreCompile.h"
#include "FontRender.h"

namespace nsK2EngineLow
{
	void FontRender::Draw(RenderContext& rc)
	{
		g_renderingEngine->AddFontRenderObject(this);
	}
}