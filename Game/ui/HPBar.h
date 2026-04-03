/**
 * HPBar.h
 * HPバーのUI
 */
#pragma once
#include "Layout.h"

class HPBarObject : public IGameObject
{
private:
	std::unique_ptr<app::ui::Layout> layout_;

	int currentHP_ = 8;


public:
	HPBarObject();
	~HPBarObject();
public:
	void Update();
	void Render(RenderContext& rc);

	void SetCurrentHP(const int currentHP)
	{
		currentHP_ = currentHP;
	}
};

