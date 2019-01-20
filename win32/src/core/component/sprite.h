#pragma once

#include "component.h"
#include "../asset/material.h"
#include "../asset/texture.h"
#include "../asset/shader.h"
#include "../asset/mesh.h"

class Sprite : public Component {
public:
	virtual void OnAdd();
	virtual void OnDel();
	virtual void OnUpdate(float dt);

	void SetMaterial(Material * material)
	{
		_material = material;
	}

	void SetShader(Shader * shader)
	{
		_shader = shader;
	}

	void SetMesh(Mesh * mesh)
	{
		_mesh = mesh;
	}

	void SetTexture(const std::string & url);

private:
	Mesh * _mesh;
	Shader * _shader;
	Material * _material;
	Texture _texture;
};