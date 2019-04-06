#pragma once

#include "component.h"
#include "../asset/material.h"
#include "../asset/texture.h"
#include "../asset/shader.h"

class SpriteScreen : public Component {
public:
	SpriteScreen();
	~SpriteScreen();
	virtual void OnAdd() override;
	virtual void OnDel() override;
	virtual void OnUpdate(float dt) override;

	void BindShader(Shader * shader);
	void BindShader(const std::string & url);

	void ClearTexture();
	void BindTexture(const Texture & texture);
	void BindTexture(const std::string & url);

private:
	Shader * _shader;
	Material _material;
	RenderMesh _meshQuat;
};