#pragma once

#include "sprite.h"
#include "../asset/material.h"
#include "../asset/texture.h"
#include "../asset/shader.h"
#include "../asset/mesh.h"

class SpriteOutline : public Sprite {
public:
	SpriteOutline() : _width(1) {}
	virtual void OnAdd() override;
	virtual void OnDel() override;
	virtual void OnUpdate(float dt) override;

	void BindOutlineShader(Shader * shader)
	{
		_outline = shader;
	}

	void SetOutlineWidth(float width)
	{
		_width = width;
	}

private:
	float _width;
	Shader * _outline;
};