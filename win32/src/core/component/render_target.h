#pragma once

#include "component.h"
#include "../asset/texture.h"
#include "../asset/bitmap.h"
#include "../asset/bitmap_cube.h"

class RenderTarget : public Component {
public:
	enum Type {
		k2D,
		k3D,
	};
public:
	RenderTarget(std::uint32_t w, std::uint32_t h, Type type = k2D);
	RenderTarget(std::uint32_t w, std::uint32_t h, GLuint bit, Type type = k2D);
	~RenderTarget();
	virtual void OnAdd() override;
	virtual void OnDel() override;
	virtual void OnUpdate(float dt) override;
	Texture GetColorTex(bool free = false);
	Texture GetDepthTex(bool free = false);
	std::uint32_t GetW() const;
	std::uint32_t GetH() const;
	bool IsColorEmpty() const;
	bool IsDepthEmpty() const;
	void Beg(size_t idx = GL_TEXTURE_2D);
	void End();

private:
	Type _type;
	GLuint _bit;
	GLuint _fbo;
	Bitmap * _color;
	Bitmap * _depth;
	BitmapCube * _colorCube;
	BitmapCube * _depthCube;
	std::uint32_t _w;
	std::uint32_t _h;
};