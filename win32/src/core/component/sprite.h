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
		BlendFunc() : mSrc(GL_ONE), mDst(GL_ZERO) { }
		BlendFunc(GLenum src, GLenum dst): mSrc(src), mDst(dst) { }
	};
public:
	Sprite();
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

	void SetFlipUVX(bool isTrue)
	{
		_flipUVX = isTrue ? 1.0f : 0.0f;
	}

	void SetFlipUVY(bool isTrue)
	{
		_flipUVY = isTrue ? 1.0f : 0.0f;
	}

	bool GetFlipUVX() const
	{
		return _flipUVX != 0.0f;
	}

	bool GetFlipUVY() const
	{
		return _flipUVY != 0.0f;
	}

	void ShowNormal(bool isTrue);

protected:
	float _flipUVX;
	float _flipUVY;
	BlendFunc _blend;
	Shader * _shader;
	Shader * _showNormal;
	std::vector<Mesh *> _meshs;
	std::vector<Material> _materials;
};