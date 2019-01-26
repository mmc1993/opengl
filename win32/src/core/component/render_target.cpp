#include "render_target.h"
#include "../tools/debug_tool.h"

RenderTarget::RenderTarget(std::uint32_t w, std::uint32_t h)
	: _w(w)
	, _h(h)
	, _fbo(0)
	, _color(nullptr)
	, _depthStencil(nullptr)
{ }

RenderTarget::~RenderTarget()
{
	glDeleteFramebuffers(1, &_fbo);
	delete _depthStencil;
	delete _color;
}

void RenderTarget::OnAdd()
{
}

void RenderTarget::OnDel()
{
}

void RenderTarget::OnUpdate(float dt)
{
}

void RenderTarget::Beg()
{
	if (_fbo == 0)
	{
		glGenFramebuffers(1, &_fbo);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	if (_color == nullptr)
	{
		_color = new Bitmap(_w, _h, GL_RGBA, "RenderTarget", nullptr);
		_color->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		_color->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _color->GetGLID(), 0);
	}
	if (_depthStencil == nullptr)
	{
		_depthStencil = new Bitmap(_w, _h, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, "RenderTarget", nullptr);
		_depthStencil->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		_depthStencil->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, _depthStencil->GetGLID(), 0);
	}
	ASSERT_RET(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

void RenderTarget::End()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::uint32_t RenderTarget::GetW() const
{
	return _w;
}

std::uint32_t RenderTarget::GetH() const
{
	return _h;
}

Texture RenderTarget::GetColorTex(bool free)
{
	auto texture = Texture(_color);
	if (free) { _color = nullptr; }
	return texture;
}

Texture RenderTarget::GetDepthStencilTex(bool free)
{
	auto texture = Texture(_depthStencil);
	if (free) { _depthStencil = nullptr; }
	return texture;
}
