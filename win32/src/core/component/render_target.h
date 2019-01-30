#pragma once

#include "component.h"
#include "../asset/texture.h"

class RenderTarget : public Component {
public:
	RenderTarget(std::uint32_t w, std::uint32_t h);
	RenderTarget(std::uint32_t w, std::uint32_t h, GLuint bit);
	~RenderTarget();
	virtual void OnAdd() override;
	virtual void OnDel() override;
	virtual void OnUpdate(float dt) override;
	Texture GetColorTex(bool free = false);
	Texture GetDepthTex(bool free = false);
	std::uint32_t GetW() const;
	std::uint32_t GetH() const;
	void Beg();
	void End();

private:
	GLuint _bit;
	GLuint _fbo;
	Bitmap * _color;
	Bitmap * _depth;
	std::uint32_t _w;
	std::uint32_t _h;
};