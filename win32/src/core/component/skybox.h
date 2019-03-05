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
	
	void BindShader(const std::string & url);
	Shader * GetShader() { return _shader; }
	
	void BindBitmapCube(const BitmapCube * cube);
	void BindBitmapCube(const std::string & url);
	const BitmapCube * GetBitmapCube() { return _bitmapCube; }

private:
	GLuint _vao;
	GLuint _vbo;
	GLuint _ebo;
	Shader * _shader;
	const BitmapCube * _bitmapCube;
};