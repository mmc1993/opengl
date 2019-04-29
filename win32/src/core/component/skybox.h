#pragma once

#include "component.h"
#include "../res/mesh.h"
#include "../res/shader.h"
#include "../res/material.h"
#include "../res/bitmap_cube.h"

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
    Mesh _meshCube;
    Shader * _shader;
	Material _material;
};