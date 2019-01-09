#pragma once

#include "component.h"
#include "../asset/material.h"

class Sprite : public Component {
public:
	virtual void OnAdd();
	virtual void OnDel();
	virtual void OnUpdate(float dt);

	void SetMaterial(Material * material)
	{
		_material = material;
	}

	Material * GetMaterial()
	{
		return _material;
	}

private:
	Material * _material;
};