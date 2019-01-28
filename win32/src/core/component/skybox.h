#pragma once

#include "component.h"
#include "../asset/shader.h"
#include "../asset/texture.h"
#include "../asset/bitmap_cube.h"

class Skybox : public Component {
public:
	Skybox();
	~Skybox();
	virtual void OnAdd() override;
	virtual void OnDel() override;
	virtual void OnUpdate(float dt) override;

	void SetBitmapCube(BitmapCube *bitmapCube)
	{
		_bitmapCube = bitmapCube;
	}

	BitmapCube * GetBitmapCube()
	{
		return _bitmapCube;
	}

	void SetShader(Shader * shader)
	{
		_shader = shader;
	}

private:
	GLuint _vao;
	GLuint _vbo;
	GLuint _ebo;
	Shader * _shader;
	BitmapCube * _bitmapCube;
};