#pragma once

#include "component.h"
#include "../asset/material.h"
#include "../asset/texture.h"
#include "../asset/shader.h"
#include "../asset/mesh.h"

class Sprite : public Component {
public:
	struct BlendFunc {
		GLenum mSrc;
		GLenum mDst;
		BlendFunc() : mSrc(GL_ONE), mDst(GL_ZERO)
		{ }
		BlendFunc(GLenum src, GLenum dst): mSrc(src), mDst(dst)
		{ }
	};
public:
	virtual ~Sprite() {}
	virtual void OnAdd() override;
	virtual void OnDel() override;
	virtual void OnUpdate(float dt) override;

	void AddMesh(Mesh * mesh, const Material & material)
	{
		_meshs.push_back(mesh);
		_materials.push_back(material);
	}

	void SetShader(Shader * shader)
	{
		_shader = shader;
	}

	void SetBlendFunc(const BlendFunc & blend)
	{
		_blend = blend;
	}

	const BlendFunc & GetBlendFunc() const
	{
		return _blend;
	}

protected:
	BlendFunc _blend;
	Shader * _shader;
	std::vector<Mesh *> _meshs;
	std::vector<Material> _materials;
};