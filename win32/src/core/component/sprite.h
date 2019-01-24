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

	void AddMesh(Mesh * mesh, const Material & material)
	{
		_meshs.push_back(mesh);
		_materials.push_back(material);
	}

	void SetShader(Shader * shader)
	{
		_shader = shader;
	}

private:
	Shader * _shader;
	std::vector<Mesh *> _meshs;
	std::vector<Material> _materials;
};