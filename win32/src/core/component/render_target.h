#pragma once

#include "component.h"
#include "../asset/texture.h"

class RenderTarget : public Component {
public:
	RenderTarget(std::uint32_t w, std::uint32_t h);
	~RenderTarget();
	virtual void OnAdd() override;
	virtual void OnDel() override;
	virtual void OnUpdate(float dt) override;
	void Beg();
	void End();
	std::uint32_t GetW() const;
	std::uint32_t GetH() const;
	Texture GetColorTex(bool free = false);
	Texture GetDepthStencilTex(bool free = false);

private:
	GLuint _fbo;
	std::uint32_t _w;
	std::uint32_t _h;
	Bitmap * _color;
	Bitmap * _depthStencil;
};