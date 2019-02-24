#pragma once

#include "component.h"
#include "../asset/material.h"
#include "../asset/texture.h"
#include "../asset/shader.h"
#include "../asset/mesh.h"

class SpriteScreen : public Component {
public:
	SpriteScreen();
	~SpriteScreen();
	virtual void OnAdd() override;
	virtual void OnDel() override;
	virtual void OnUpdate(float dt) override;

	bool GetFlipUVX() const { return _flipUVX != 0.0f; }
	bool GetFlipUVY() const { return _flipUVY != 0.0f; }
	void SetFlipUVX(bool isTrue) { _flipUVX = isTrue ? 1.0f : 0.0f; }
	void SetFlipUVY(bool isTrue) { _flipUVY = isTrue ? 1.0f : 0.0f; }
	void BindTexture(const Texture & texture) { _texture = texture; }
	void BindShader(Shader * shader) { _shader = shader; }

private:
	float _flipUVX;
	float _flipUVY;
	Mesh _meshQuat;
	Shader * _shader;
	Texture _texture;
};