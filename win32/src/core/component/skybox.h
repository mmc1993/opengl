#pragma once

#include "component.h"
#include "../asset/shader.h"
#include "../asset/material.h"
#include "../asset/bitmap_cube.h"

class Skybox : public Component {
public:
	Skybox();
	~Skybox();
	virtual void OnAdd() override;
	virtual void OnDel() override;
	virtual void OnUpdate(float dt) override;
	
	void BindShader(Shader * shader);
	void BindShader(const std::string & url);
	
	void BindTexture(BitmapCube * texture);
	void BindTexture(const std::string & url);
	const BitmapCube * GetTexture() { return _material.mTexCube; }

private:
	Shader * _shader;
	Material _material;
	RenderMesh _meshCube;
};